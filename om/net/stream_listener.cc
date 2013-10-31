//
//  Olli's C++ Library [https://bitbucket.org/omichel/om-lib]
//  net/stream_listener.cc
//  (c) 2013 Oliver Michel <oliver dot michel at editum dot de>
//

#include "stream_listener.h"

om::net::StreamListener::StreamListener()
	: om::net::Socket(om::net::IOInterface::iface_type_sock_stream) {}

om::net::StreamListener::StreamListener(const om::net::tp_addr addr)
	throw(std::runtime_error, std::invalid_argument)
	: om::net::Socket(om::net::IOInterface::iface_type_sock_stream) {

	this->open(addr);
}

om::net::StreamListener::StreamListener(const om::net::StreamListener &copy_from)
	: om::net::Socket(copy_from), _addr(copy_from._addr) {}

om::net::StreamListener& om::net::StreamListener::operator=(StreamListener& copy_from) {

	om::net::Socket::operator=(copy_from);
	_addr = copy_from._addr;
	return *this;
}

int om::net::StreamListener::open(const om::net::tp_addr addr)
	throw(std::runtime_error, std::logic_error, std::invalid_argument) {

	if(_fd != 0) 
   	throw std::logic_error("Socket already opened");
  
  	if(addr.proto != om::net::tp_proto_tcp)
   	throw std::invalid_argument("ip_endpoint must be a TCP endpoint");

	int fd, yes = 1;
	struct sockaddr_in addr_struct;

	// create new stream socket
	if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		::close(fd);
		throw std::runtime_error("socket(): " + std::string(strerror(errno)));
	}
		
	// reuse socket address if in use
	if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0) {
		::close(fd);
		throw std::runtime_error("setsockopt(): " + std::string(strerror(errno)));
	}
		
	// fill sockaddr_in struct
	om::net::sockaddr_from_tp_addr(addr, &addr_struct);

	// bind newly created socket to address
	if(bind(fd, (struct sockaddr*)&addr_struct, sizeof(addr_struct)) < 0) {
		::close(fd);
		throw std::runtime_error("bind(): " + std::string(strerror(errno)));
	}
		
	// begin listening
	if(listen(fd, PENDING_CONN_QLEN) < 0) {
		::close(fd);
		throw std::runtime_error("bind(): " + std::string(strerror(errno)));
	}
	
	_addr = addr;
	_fd = fd;

	return fd;
}

int om::net::StreamListener::accept(om::net::tp_addr* remote_addr)
	throw(std::runtime_error) {

	int incoming_fd = -1;

	struct sockaddr_in addr_struct;
	socklen_t addr_struct_len = sizeof(struct sockaddr_in);
	
	// accept new incoming connection and return new fd
	if((incoming_fd = ::accept(_fd, (sockaddr*)&addr_struct, &addr_struct_len)) < 0)
		throw std::runtime_error("accept(): " + std::string(strerror(errno)));	

	if(remote_addr) {
		om::net::tp_addr_from_sockaddr(&addr_struct, remote_addr);
		remote_addr->proto = om::net::tp_proto_tcp;
	}
		
	return incoming_fd;
}

void om::net::StreamListener::close()
	throw(std::logic_error, std::runtime_error) {
	
	if(_fd == 0)
		throw std::logic_error("Socket was already ::closed or never opened");

	if(::close(_fd) == 0)
		_fd = 0;
	else
		throw std::runtime_error("::close(): " + std::string(strerror(errno)));
}

om::net::StreamListener::~StreamListener() {}