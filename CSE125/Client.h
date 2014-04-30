#pragma once
#include <string>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include "object.h"
#include "glslprogram.h" //Mat4 data type

using boost::asio::ip::tcp;

class tcp_client
{
public:
	tcp_client(boost::asio::io_service& io_service, const std::string& host, const std::string& port)
		: io_service_(io_service), socket_(io_service)
	{
		tcp::resolver resolver(io_service);
		tcp::resolver::query query(tcp::v4(), host, port);
		tcp::resolver::iterator itr = resolver.resolve(query);

		endpoint_ = *itr;
		std::cout << "Address: " << endpoint_.address() << std::endl;

		ret_.push_back(std::make_pair("initRet_c", mat4(0.0f)));
		ret_.push_back(std::make_pair("initRet_c", mat4(0.0f)));
		ret_.push_back(std::make_pair("initRet_c", mat4(0.0f)));
		ret_.push_back(std::make_pair("initRet_c", mat4(0.0f)));
		pIDVec_.push_back(std::make_pair("initPID_c", mat4(0.0f)));

		socket_.async_connect(endpoint_,
			boost::bind(&tcp_client::handle_connect, this,
			boost::asio::placeholders::error));
	}

	~tcp_client()
	{
		socket_.close();
	}

	void write(std::vector <std::pair<string, mat4>>& in)
	{
		std::cout << "write called" << std::endl;
		std::cout << "sendVec string:" << in.front().first << std::endl;
		/*std::cout << "sendVec dat:" << in.front().second[0][0] <<
			in.front().second[0][1] <<
			in.front().second[0][2] <<
			in.front().second[0][3] <<
			in.front().second[1][0] <<
			in.front().second[1][1] <<
			in.front().second[1][2] <<
			in.front().second[1][3] << std::endl;*/
		boost::asio::async_write(socket_,
			boost::asio::buffer(in, 2048),
			boost::bind(&tcp_client::handle_write, this,
			boost::asio::placeholders::error));
	}


	std::vector <std::pair<string, mat4>>* read()
	{
		std::cout << "read called" << std::endl;
		std::cout << "recvVec string:" << ret_.front().first << std::endl;
		/*std::cout << "recvVec dat:" << ret_.front().second[0][0] <<
			ret_.front().second[0][1] <<
			ret_.front().second[0][2] <<
			ret_.front().second[0][3] <<
			ret_.front().second[1][0] <<
			ret_.front().second[1][1] <<
			ret_.front().second[1][2] <<
			ret_.front().second[1][3] <<
			ret_.front().second[2][0] <<
			ret_.front().second[2][1] <<
			ret_.front().second[2][2] <<
			ret_.front().second[2][3] <<
			ret_.front().second[3][0] <<
			ret_.front().second[3][1] <<
			ret_.front().second[3][2] <<
			ret_.front().second[3][3] << std::endl;*/
		return &ret_;
	}

	int pID()
	{
		while (!pID_ready)
		{
			Sleep(1);
		}
		return atoi(pIDVec_.front().first.c_str());
	}

private:
	boost::asio::io_service& io_service_;
	tcp::socket socket_;
	tcp::endpoint endpoint_;
	std::vector <std::pair<string, mat4>> ret_; // possible concurrence problems
	std::vector <std::pair<string, mat4>> pIDVec_;
	bool pID_ready = 0;

	void handle_connect(const boost::system::error_code& error)
	{
		if (!error)
		{
			std::cout << "Client connected to server" << std::endl;
			boost::asio::async_read(socket_,
				boost::asio::buffer(pIDVec_, 2048),
				boost::bind(&tcp_client::handle_PID, this,
				boost::asio::placeholders::error));
		}
		else
		{
			std::cout << "Error " << error << std::endl;
			socket_.close();
		}
	}

	void handle_PID(const boost::system::error_code& error)
	{
		if (!error)
		{
			pID_ready = 1;
			std::cout << "Reading" << std::endl;
			boost::asio::async_read(socket_,
				boost::asio::buffer(ret_, 2048),
				boost::bind(&tcp_client::handle_read, this,
				boost::asio::placeholders::error));
		}
		else
		{
			std::cout << "Error " << error << std::endl;
			socket_.close();
		}
	}

	void handle_read(const boost::system::error_code& error)
	{
		if (!error)
		{
			/*::cout << "read called" << std::endl;
			std::cout << "recvVec string:" << ret_.front().first << std::endl;
			std::cout << "recvVec dat:" << ret_.front().second[0][0] <<
				ret_.front().second[0][1] <<
				ret_.front().second[0][2] <<
				ret_.front().second[0][3] <<
				ret_.front().second[1][0] <<
				ret_.front().second[1][1] <<
				ret_.front().second[1][2] <<
				ret_.front().second[1][3] <<
				ret_.front().second[2][0] <<
				ret_.front().second[2][1] <<
				ret_.front().second[2][2] <<
				ret_.front().second[2][3] <<
				ret_.front().second[3][0] <<
				ret_.front().second[3][1] <<
				ret_.front().second[3][2] <<
				ret_.front().second[3][3] << std::endl; */

			std::cout << "Reading" << std::endl;
			boost::asio::async_read(socket_,
				boost::asio::buffer(ret_, 2048),
				boost::bind(&tcp_client::handle_read, this,
				boost::asio::placeholders::error));
		}
		else
		{
			std::cout << "Error " << error << std::endl;
			socket_.close();
		}
	}

	/*void write(std::vector <pair<string, mat4>>& in)
	{
		boost::asio::async_write(socket_,
			boost::asio::buffer(in, 2048),
			boost::bind(&tcp_client::handle_write, this,
			boost::asio::placeholders::error));
	}*/

	void handle_write(const boost::system::error_code& error)
	{
		if (!error)
		{
			std::cout << "sent data" << std::endl;
		}
		else
		{
			std::cout << "Error" << std::endl;
			socket_.close();
		}
	}
};