#include "PythonCommunicator.h"
#include <utils/Common.h>
#include <string>
#include <boost/python/extract.hpp>

SMI::Utils::PythonCommunicator::PythonCommunicator(SMI::Utils::peer_num peer_id, SMI::Utils::peer_num num_peers, std::string config_path,
                                                   std::string comm_name, unsigned int faas_memory) {
    comm = std::make_shared<SMI::Communicator>(peer_id, num_peers, config_path, comm_name, faas_memory);
    this->peer_id = peer_id;
    this->num_peers = num_peers;
}

void SMI::Utils::PythonCommunicator::send(const boost::python::object& py_obj, SMI::Utils::peer_num dst, SMI::Utils::PythonData type) {
    if (type.type == INT) {
        auto val = extract_object<int>(py_obj);
        SMI::Comm::Data<int> data = val;
        comm->send<int>(data, dst);
    } else if (type.type == DOUBLE) {
        auto val = extract_object<double>(py_obj);
        SMI::Comm::Data<double> data = val;
        comm->send<double>(data, dst);
    } else if (type.type == INT_LIST) {
        auto val = extract_list<int>(py_obj);
        SMI::Comm::Data<std::vector<int>> data = val;
        comm->send<std::vector<int>>(data, dst);
    } else if (type.type == DOUBLE_LIST) {
        auto val = extract_list<double>(py_obj);
        SMI::Comm::Data<std::vector<double>> data = val;
        comm->send<std::vector<double>>(data, dst);
    } else {
        throw "Unknown type passed";
    }
}

boost::python::object SMI::Utils::PythonCommunicator::recv(SMI::Utils::peer_num src, SMI::Utils::PythonData type) {
    if (type.type == INT) {
        SMI::Comm::Data<int> data;
        comm->recv<int>(data, src);
        boost::python::object res(data.get());
        return res;
    } else if (type.type == DOUBLE) {
        SMI::Comm::Data<double> data;
        comm->recv<double>(data, src);
        boost::python::object res(data.get());
        return res;
    } else if (type.type == INT_LIST) {
        SMI::Comm::Data<std::vector<int>> data(type.num_objects);
        comm->recv<std::vector<int>>(data, src);
        boost::python::object res(to_list<int>(data.get()));
        return res;
    } else if (type.type == DOUBLE_LIST) {
        SMI::Comm::Data<std::vector<double>> data(type.num_objects);
        comm->recv<std::vector<double>>(data, src);
        boost::python::object res(to_list<double>(data.get()));
        return res;
    } else {
        throw "Unknown type passed";
    }
}

boost::python::object SMI::Utils::PythonCommunicator::bcast(const boost::python::object& src_data, SMI::Utils::peer_num root, SMI::Utils::PythonData type) {
    if (type.type == INT) {
        SMI::Comm::Data<int> data;
        if (peer_id == root) {
            auto val = extract_object<int>(src_data);
            data = val;
        }
        comm->bcast<int>(data, root);
        return boost::python::object(data.get());
    } else if (type.type == DOUBLE) {
        SMI::Comm::Data<double> data;
        if (peer_id == root) {
            auto val = extract_object<double>(src_data);
            data = val;
        }
        comm->bcast<double>(data, root);
        return boost::python::object(data.get());
    } else if (type.type == INT_LIST) {
        SMI::Comm::Data<std::vector<int>> data(type.num_objects);
        if (peer_id == root) {
            auto val = extract_list<int>(src_data);
            data = val;
        }
        comm->bcast<std::vector<int>>(data, root);
        return boost::python::object(to_list<int>(data.get()));
    } else if (type.type == DOUBLE_LIST) {
        SMI::Comm::Data<std::vector<double>> data(type.num_objects);
        if (peer_id == root) {
            auto val = extract_list<double>(src_data);
            data = val;
        }
        comm->bcast<std::vector<double>>(data, root);
        return boost::python::object(to_list<double>(data.get()));
    } else {
        throw "Unknown type passed";
    }
}

void SMI::Utils::PythonCommunicator::barrier() {
    comm->barrier();
}

boost::python::object
SMI::Utils::PythonCommunicator::gather(const boost::python::object& src_data, SMI::Utils::peer_num root, SMI::Utils::PythonData snd_type) {
    if (snd_type.type == INT || snd_type.type == INT_LIST) {
        std::vector<int> val_list;
        if (snd_type.type == INT) {
            auto val = extract_object<int>(src_data);
            snd_type.num_objects = 1;
            val_list.emplace_back(val);
        } else if (snd_type.type == INT_LIST) {
            val_list = extract_list<int>(src_data);
        }
        SMI::Comm::Data<std::vector<int>> senddata(val_list);
        SMI::Comm::Data<std::vector<int>> rcvdata;
        if (peer_id == root) {
            rcvdata = SMI::Comm::Data<std::vector<int>>(snd_type.num_objects * num_peers);
        }
        comm->gather<std::vector<int>>(senddata, rcvdata, root);
        return boost::python::object(to_list<int>(rcvdata.get()));
    } else if (snd_type.type == DOUBLE || snd_type.type == DOUBLE_LIST) {
        std::vector<double> val_list;
        if (snd_type.type == DOUBLE) {
            auto val = extract_object<double>(src_data);
            snd_type.num_objects = 1;
            val_list.emplace_back(val);
        } else if (snd_type.type == DOUBLE_LIST) {
            val_list = extract_list<double>(src_data);
        }
        SMI::Comm::Data<std::vector<double>> senddata(val_list);
        SMI::Comm::Data<std::vector<double>> rcvdata;
        if (peer_id == root) {
            rcvdata = SMI::Comm::Data<std::vector<double>>(snd_type.num_objects * num_peers);
        }
        comm->gather<std::vector<double>>(senddata, rcvdata, root);
        return boost::python::object(to_list<double>(rcvdata.get()));
    } else {
        throw "Unknown type passed";
    }
}

boost::python::object
SMI::Utils::PythonCommunicator::scatter(const boost::python::object& src_data, SMI::Utils::peer_num root, SMI::Utils::PythonData snd_type) {
    if (snd_type.num_objects % num_peers != 0) {
        throw "List length not divisible by number of peers";
    }
    if (snd_type.type == INT || snd_type.type == DOUBLE) {
        throw "Cannot scatter atomic types";
    } else if (snd_type.type == INT_LIST) {
        SMI::Comm::Data<std::vector<int>> recvdata(snd_type.num_objects / num_peers);
        SMI::Comm::Data<std::vector<int>> senddata;
        if (peer_id == root) {
            auto val = extract_list<int>(src_data);
            senddata = val;
        }
        comm->scatter<std::vector<int>>(senddata, recvdata, root);
        return boost::python::object(to_list<int>(recvdata.get()));
    } else if (snd_type.type == DOUBLE_LIST) {
        SMI::Comm::Data<std::vector<double>> recvdata(snd_type.num_objects / num_peers);
        SMI::Comm::Data<std::vector<double>> senddata;
        if (peer_id == root) {
            auto val = extract_list<double>(src_data);
            senddata = val;
        }
        comm->scatter<std::vector<double>>(senddata, recvdata, root);
        return boost::python::object(to_list<double>(recvdata.get()));
    } else {
        throw "Unknown type passed";
    }
}

boost::python::object
SMI::Utils::PythonCommunicator::reduce(const boost::python::object& src_data, SMI::Utils::peer_num root, SMI::Utils::PythonFunc f,
                                       SMI::Utils::PythonData type) {
    if (type.type == INT) {
        auto val = extract_object<int>(src_data);
        SMI::Comm::Data<int> sendbuf(val);
        SMI::Comm::Data<int> recvbuf;
        SMI::Utils::Function<int> func([] (int a, int b) {return a + b;}, true, true);
        if (f.op == PROD) {
            func = SMI::Utils::Function<int>([] (int a, int b) {return a * b;}, true, true);
        } else if (f.op == MAX) {
            func = SMI::Utils::Function<int>([] (int a, int b) {return std::max(a, b);}, true, true);
        } else if (f.op == MIN) {
            func = SMI::Utils::Function<int>([] (int a, int b) {return std::min(a, b);}, true, true);
        }
        comm->reduce(sendbuf, recvbuf, root, func);
        return boost::python::object(recvbuf.get());
    } else if (type.type == DOUBLE) {
        auto val = extract_object<double>(src_data);
        SMI::Comm::Data<double> sendbuf(val);
        SMI::Comm::Data<double> recvbuf;
        SMI::Utils::Function<double> func([] (double a, double b) {return a + b;}, true, true);
        if (f.op == PROD) {
            func = SMI::Utils::Function<double>([] (double a, double b) {return a * b;}, true, true);
        } else if (f.op == MAX) {
            func = SMI::Utils::Function<double>([] (double a, double b) {return std::max(a, b);}, true, true);
        } else if (f.op == MIN) {
            func = SMI::Utils::Function<double>([] (double a, double b) {return std::min(a, b);}, true, true);
        }
        comm->reduce(sendbuf, recvbuf, root, func);
        return boost::python::object(recvbuf.get());
    }

}
