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
 * DARC Zeroconf Published Service
 *
 * \author Morten Kjaergaard
 */

#include <avahi-client/client.h>
#include <avahi-client/publish.h>
#include <avahi-common/error.h>
#include <darc/id.hpp>

#include <stdio.h>

namespace darc
{
namespace zeroconf
{

class service
{
public:
  darc::ID peer_id_;
  uint16_t port_;

  service(const darc::ID& peer_id, uint32_t port) :
    peer_id_(peer_id),
    port_(port)
  {
  }

  ~service()
  {
  }

  void create_service(AvahiClient *client, AvahiEntryGroup *group)
  {
    printf("adding service\n");
    int ret = avahi_entry_group_add_service(group,
                                            AVAHI_IF_UNSPEC,
                                            AVAHI_PROTO_INET,
                                            (AvahiPublishFlags)0,
                                            std::string("darc_peer_").append(peer_id_.short_string()).c_str(),
                                            "_darc._tcp",
                                            NULL,
                                            NULL,
                                            port_,
                                            std::string("id=").append(peer_id_.short_string()).c_str(),
                                            NULL);
    // todo: handle ret == AVAHI_ERR_COLLISION
    if (ret < 0)
    {
      fprintf(stderr, "Failed to add service: %s\n", avahi_strerror(ret));
      assert(false);
    }

    /* Tell the server to register the service */
    ret = avahi_entry_group_commit(group);
    if (ret < 0)
    {
      fprintf(stderr, "Failed to commit entry group: %s\n", avahi_strerror(ret));
      assert(false);
    }
  }

};

}
}
