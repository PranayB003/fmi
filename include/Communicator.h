#ifndef SMI_COMMUNICATOR_H
#define SMI_COMMUNICATOR_H

#include "./utils/Configuration.h"
#include "comm/Channel.h"

namespace SMI {
    class Communicator {
    public:
        Communicator(unsigned int peer_id, unsigned int num_peers, std::string config_path, bool sync = false);

        template<typename T>
        void send(Comm::Data<T> buf, peer_num dest) {
            channel_data data {buf.data(), buf.size_in_bytes()};
            channels[0]->send(data, 0);
        }

        template<typename T>
        void recv(Comm::Data<T> buf, peer_num dest);

        template <typename T>
        void reduce(Comm::Data<T> sendbuf, Comm::Data<T> recvbuf, peer_num root, SMI::Utils::Function<T> f);

        template <typename T>
        void allreduce(Comm::Data<T> sendbuf, Comm::Data<T> recvbuf, SMI::Utils::Function<T> f);

        template<typename T>
        void gather(Comm::Data<T> sendbuf, Comm::Data<T> recvbuf, peer_num root);

        template<typename T>
        void scatter(Comm::Data<T> sendbuf, Comm::Data<T> recvbuf, peer_num root);

        template<typename T>
        void bcast(Comm::Data<T> buf, peer_num root);

        template<typename T>
        void scan(Comm::Data<T> sendbuf, Comm::Data<T> recvbuf, SMI::Utils::Function<T> f);

        void register_channel(SMI::Comm::Channel &c);

    private:
        std::vector<SMI::Comm::Channel*> channels;
    };
}



#endif //SMI_COMMUNICATOR_H
