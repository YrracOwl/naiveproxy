// Copyright (c) 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/third_party/quiche/src/quic/test_tools/simple_session_cache.h"
#include <memory>
#include "net/third_party/quiche/src/quic/core/crypto/quic_crypto_client_config.h"

namespace quic {
namespace test {

void SimpleSessionCache::Insert(const QuicServerId& server_id,
                                bssl::UniquePtr<SSL_SESSION> session,
                                const TransportParameters& params,
                                const ApplicationState* application_state) {
  auto it = cache_entries_.find(server_id);
  if (it == cache_entries_.end()) {
    it = cache_entries_.insert(std::make_pair(server_id, Entry())).first;
  }
  if (session != nullptr) {
    it->second.session = std::move(session);
  }
  if (application_state != nullptr) {
    it->second.application_state =
        std::make_unique<ApplicationState>(*application_state);
  }
  it->second.params = std::make_unique<TransportParameters>(params);
}

std::unique_ptr<QuicResumptionState> SimpleSessionCache::Lookup(
    const QuicServerId& server_id,
    const SSL_CTX* /*ctx*/) {
  auto it = cache_entries_.find(server_id);
  if (it == cache_entries_.end()) {
    return nullptr;
  }

  if (!it->second.session) {
    cache_entries_.erase(it);
    return nullptr;
  }

  auto state = std::make_unique<QuicResumptionState>();
  state->tls_session = std::move(it->second.session);
  if (it->second.application_state != nullptr) {
    state->application_state =
        std::make_unique<ApplicationState>(*it->second.application_state);
  }
  state->transport_params =
      std::make_unique<TransportParameters>(*it->second.params);
  return state;
}

void SimpleSessionCache::ClearEarlyData(const QuicServerId& /*server_id*/) {
  // The simple session cache only stores 1 SSL ticket per entry, so no need to
  // do anything here.
}

}  // namespace test
}  // namespace quic
