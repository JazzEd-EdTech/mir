/*
 * Copyright © 2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alexandros Frantzis <alexandros.frantzis@canonical.com>
 */

#ifndef MIR_CLIENT_LOGGING_RPC_REPORT_H_
#define MIR_CLIENT_LOGGING_RPC_REPORT_H_

#include "../rpc/rpc_report.h"

#include <memory>

namespace mir
{

namespace logging
{
class Logger;
}

namespace client
{
namespace logging
{

class RpcReport : public rpc::RpcReport
{
public:
    RpcReport(std::shared_ptr<mir::logging::Logger> const& logger);

    void invocation_requested(mir::protobuf::wire::Invocation const& invocation) override;
    void invocation_succeeded(mir::protobuf::wire::Invocation const& invocation) override;
    void invocation_failed(mir::protobuf::wire::Invocation const& invocation,
                           boost::system::error_code const& error) override;

    void header_receipt_failed(boost::system::error_code const& error) override;
    void result_receipt_succeeded(mir::protobuf::wire::Result const& result) override;
    void result_receipt_failed(std::exception const& ex) override;

    void event_parsing_succeeded(MirEvent const& event) override;
    void event_parsing_failed(mir::protobuf::Event const& event) override;

    void orphaned_result(mir::protobuf::wire::Result const& result) override;
    void complete_response(mir::protobuf::wire::Result const& result) override;

    void result_processing_failed(mir::protobuf::wire::Result const& result,
                                  std::exception const& ex) override;

    void file_descriptors_received(google::protobuf::Message const& response,
                                   std::vector<int32_t> const& fds) override;

    void connection_failure(std::exception const& ex) override;

private:
    std::shared_ptr<mir::logging::Logger> const logger;
};

}
}
}

#endif /* MIR_CLIENT_LOGGING_RPC_REPORT_H_ */
