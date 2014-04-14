#include <string>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include "Object.h"
#include "glslprogram.h" //Mat4 data type

using boost::asio::ip::udp;

class udpClient
{

public:
	udpClient(boost::asio::io_service& io_service, const std::string& host, const std::string& port)
		: io_service_(io_service), socket_(io_service, udp::endpoint(udp::v4(), 0)) {
		udp::resolver resolver(io_service);
		udp::resolver::query query(udp::v4(), host, port);
		udp::resolver::iterator itr = resolver.resolve(query);

		remote_endpoint_ = *itr;

		// Possibly send some kind of init message to server on this initial send
		start_send();
	}

	~udpClient()
	{
		socket_.close();
	}

	void send_keyState(int keyState)
	{
		send_buf_[0] = keyState;
		start_send();
	}
	int get_keyState()
	{
		return recv_buf_[0];
	}

	boost::array<mat4, 1> get_PosUpdate()
	{
		listen_posUpdate();
		return recv_mat_;
	}

private:
	boost::asio::io_service& io_service_;
	udp::socket socket_;
	enum { max_length = 1024 };
	char data_[max_length];

	boost::array<int, 1> send_buf_ = { { 0 } };
	boost::array<int, 1> recv_buf_ = { { 0 } };
	boost::array<mat4, 1> recv_mat_;

	udp::endpoint remote_endpoint_;

	void start_send()
	{
		//std::cout << "Sending: " << send_buf_.data() << std::endl;

		boost::shared_ptr<std::string> message(
			new std::string("this is a string"));

		socket_.async_send_to(
			boost::asio::buffer(send_buf_), remote_endpoint_,
			boost::bind(&udpClient::handle_send, this, message,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	}

	void listen_posUpdate()
	{
		socket_.async_receive_from(boost::asio::buffer(recv_mat_), remote_endpoint_,
			boost::bind(&udpClient::handle_recv_pos, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	}

	void handle_receive(const boost::system::error_code& error,
		std::size_t len)
	{
		//std::cout << "Receiving: " << std::endl;
		//player_list[0]->setModelM(recv_mat_[0]);
	}

	void handle_recv_pos(const boost::system::error_code& error,
		std::size_t len)
	{
		socket_.async_receive_from(boost::asio::buffer(recv_mat_), remote_endpoint_,
			boost::bind(&udpClient::handle_recv_pos, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	}

	void handle_send(boost::shared_ptr<std::string> /*message*/,
		const boost::system::error_code& error,
		std::size_t /*bytes_transferred*/)
	{
		/*if (!error || error == boost::asio::error::message_size)
		{
		socket_.async_receive_from(boost::asio::buffer(recv_buf_), remote_endpoint_,
		boost::bind(&udp_client::handle_receive, this,
		boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred));
		}*/
	}
};