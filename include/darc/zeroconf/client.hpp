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

#include <list>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/function.hpp>
#include <avahi-client/client.h>
#include <avahi-client/publish.h>
#include <avahi-client/lookup.h>
#include <avahi-common/simple-watch.h>
#include <avahi-common/error.h>
#include <darc/zeroconf/service.hpp>

namespace darc
{
namespace zeroconf
{

class client
{
public:
  typedef boost::function<void(const std::string&, int)> service_callback_type;

protected:
  AvahiSimplePoll *poll_;
  AvahiEntryGroup *group_;
  AvahiClient *client_;
  AvahiServiceBrowser *browser_;
  service_callback_type service_callback_;

  typedef std::list<boost::shared_ptr<service> > service_list_type;
  service_list_type service_list_;

public:
  client(service_callback_type service_callback);
  ~client();

  void run();
  void add_service(const darc::ID& peer_id, uint16_t port);
  void client_callback(AvahiClient *c, AvahiClientState state);
  void browse_callback(AvahiServiceBrowser *b,
                       AvahiIfIndex interface,
                       AvahiProtocol protocol,
                       AvahiBrowserEvent event,
                       const char *name,
                       const char *type,
                       const char *domain,
                       AvahiLookupResultFlags flags);

  void resolve_callback(AvahiServiceResolver *r,
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
                        AvahiLookupResultFlags flags);

};


static void trigger_client_callback(AvahiClient *c, AvahiClientState state, void * userdata)
{
  static_cast<client*>(userdata)->client_callback(c, state);
}

static void trigger_browse_callback(AvahiServiceBrowser *b,
                                    AvahiIfIndex interface,
                                    AvahiProtocol protocol,
                                    AvahiBrowserEvent event,
                                    const char *name,
                                    const char *type,
                                    const char *domain,
                                    AvahiLookupResultFlags flags,
                                    void* userdata)
{
  static_cast<client*>(userdata)->browse_callback(b,
      interface,
      protocol,
      event,
      name,
      type,
      domain,
      flags);
}

static void trigger_resolve_callback(AvahiServiceResolver *r,
                                     AvahiIfIndex interface,
                                     AvahiProtocol protocol,
                                     AvahiResolverEvent event,
                                     const char *name,
                                     const char *type,
                                     const char *domain,
                                     const char *host_name,
                                     const AvahiAddress *address,
                                     uint16_t port,
                                     AvahiStringList *txt,
                                     AvahiLookupResultFlags flags,
                                     void* userdata)
{
  static_cast<client*>(userdata)->resolve_callback(r,
      interface,
      protocol,
      event,
      name,
      type,
      domain,
      host_name,
      address,
      port,
      txt,
      flags);
}

static void trigger_group_callback(AvahiEntryGroup *g, AvahiEntryGroupState state, void *userdata)
{
}

}
}
