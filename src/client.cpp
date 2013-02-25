/*
 * Copyright (c) 2013, Prevas A/S
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Prevas A/S nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * DARC Zeroconf Client interface
 *
 * \author Morten Kjaergaard
 */

#include <darc/zeroconf/client.hpp>

namespace darc
{
namespace zeroconf
{

client::client(service_callback_type service_callback) :
        poll_(avahi_simple_poll_new()),
        group_(0),
        service_callback_(service_callback)
{
  int error;
  client_ = avahi_client_new(avahi_simple_poll_get(poll_),
      (AvahiClientFlags)0, &trigger_client_callback, (void*)this, &error);
  assert(client_);

  browser_ = avahi_service_browser_new(client_,
      AVAHI_IF_UNSPEC,
      AVAHI_PROTO_UNSPEC,
      "_darc._tcp",
      NULL,
      (AvahiLookupFlags)0,
      &trigger_browse_callback,
      this);
  assert(browser_);
}

client::~client()
{
  if (client_)
  {
    avahi_client_free(client_);
  }

  if(poll_)
  {
    avahi_simple_poll_free(poll_);
  }
}

void client::run()
{
  int ret = avahi_simple_poll_loop(poll_);
  printf("poll %i", ret);
}

void client::add_service(const darc::ID& peer_id, uint16_t port)
{
  service_list_.push_back(boost::make_shared<service>(peer_id, port));
  if(group_)
  {
    service_list_.back()->create_service(client_, group_);
  }
}

void client::client_callback(AvahiClient *c, AvahiClientState state)
{
  assert(c);

  /* Called whenever the client or server state changes */

  switch (state)
  {
  case AVAHI_CLIENT_S_RUNNING:
    if (!group_)
    {
      group_ = avahi_entry_group_new(c, &trigger_group_callback, NULL);
      assert(group_);
    }

    /* If the group is empty (either because it was just created, or
     * because it was reset previously, add our entries.  */

    if (avahi_entry_group_is_empty(group_))
    {
      /* The server has startup successfully and registered its host
       * name on the network, so it's time to create our services */
      for(service_list_type::iterator it = service_list_.begin();
          it != service_list_.end();
          it++)
      {
        (*it)->create_service(client_, group_);
      }
    }
    break;

  case AVAHI_CLIENT_FAILURE:

    fprintf(stderr, "Client failure: %s\n", avahi_strerror(avahi_client_errno(c)));
    assert(false);

    break;

  case AVAHI_CLIENT_S_COLLISION:

    /* Let's drop our registered services. When the server is back
     * in AVAHI_SERVER_RUNNING state we will register them
     * again with the new host name. */

  case AVAHI_CLIENT_S_REGISTERING:

    /* The server records are now being established. This
     * might be caused by a host name change. We need to wait
     * for our own records to register until the host name is
     * properly esatblished. */

    if (group_)
      avahi_entry_group_reset(group_);

    break;

  case AVAHI_CLIENT_CONNECTING:
    break;
  }
}

void client::browse_callback(AvahiServiceBrowser *b,
                             AvahiIfIndex interface,
                             AvahiProtocol protocol,
                             AvahiBrowserEvent event,
                             const char *name,
                             const char *type,
                             const char *domain,
                             AvahiLookupResultFlags flags)
{
  assert(b);

  switch (event)
  {
  case AVAHI_BROWSER_FAILURE:
  {
    assert(false);
    return;
  }
  case AVAHI_BROWSER_NEW:
  {
    printf("(Browser) NEW: service '%s' of type '%s' in domain '%s'\n", name, type, domain);
    AvahiServiceResolver* r = avahi_service_resolver_new(client_,
                                                         interface,
                                                         protocol,
                                                         name,
                                                         type,
                                                         domain,
                                                         AVAHI_PROTO_UNSPEC,
                                                         (AvahiLookupFlags)0,
                                                         &trigger_resolve_callback,
                                                         this);
    break;
  }
  case AVAHI_BROWSER_REMOVE:
  {
    printf("(Browser) REMOVE: service '%s' of type '%s' in domain '%s'\n", name, type, domain);
    break;
  }
  case AVAHI_BROWSER_ALL_FOR_NOW:
  case AVAHI_BROWSER_CACHE_EXHAUSTED:
  {
    printf("(Browser) %s\n", event == AVAHI_BROWSER_CACHE_EXHAUSTED ? "CACHE_EXHAUSTED" : "ALL_FOR_NOW");
    break;
  }
  }
}

void client::resolve_callback(AvahiServiceResolver *r,
                              AVAHI_GCC_UNUSED AvahiIfIndex interface,
                              AVAHI_GCC_UNUSED AvahiProtocol protocol,
                              AvahiResolverEvent event,
                              const char *name,
                              const char *type,
                              const char *domain,
                              const char *host_name,
                              const AvahiAddress *address,
                              uint16_t port,
                              AvahiStringList *txt,
                              AvahiLookupResultFlags flags)
{
  assert(r);

  switch (event)
  {
  case AVAHI_RESOLVER_FAILURE:
    printf("(Resolver) Failed to resolve service '%s' of type '%s' in domain '%s': %s\n",
        name,
        type,
        domain,
        avahi_strerror(avahi_client_errno(avahi_service_resolver_get_client(r))));
    break;

  case AVAHI_RESOLVER_FOUND:
  {
    char a[AVAHI_ADDRESS_STR_MAX], *t;

    printf("Service '%s' of type '%s' in domain '%s':\n", name, type, domain);

    if(!(flags & AVAHI_LOOKUP_RESULT_OUR_OWN) &&
        (flags & AVAHI_LOOKUP_RESULT_LOCAL))
    {
      service_callback_(std::string(host_name), port);
    }

    avahi_address_snprint(a, sizeof(a), address);
    /*  t = avahi_string_list_to_string(txt);
    printf("\t%s:%u (%s)\n"
           "\tTXT=%s\n"
           "\tcookie is %u\n"
           "\tis_local: %i\n"
           "\tour_own: %i\n"
           "\twide_area: %i\n"
           "\tmulticast: %i\n"
            "\tcached: %i\n",
           host_name, port, a,
           t,
           avahi_string_list_get_service_cookie(txt),
           !!(flags & AVAHI_LOOKUP_RESULT_LOCAL),
           !!(flags & AVAHI_LOOKUP_RESULT_OUR_OWN),
           !!(flags & AVAHI_LOOKUP_RESULT_WIDE_AREA),
           !!(flags & AVAHI_LOOKUP_RESULT_MULTICAST),
           !!(flags & AVAHI_LOOKUP_RESULT_CACHED));

    avahi_free(t);
     */
  }
  }

  avahi_service_resolver_free(r);
}

}
}
