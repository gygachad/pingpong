#include "connection.h"

bool connection::connect(const std::string& ip, const uint16_t port)
{
	asio::ip::tcp::endpoint ep(asio::ip::address::from_string(ip), port);
	m_sock = asio::ip::tcp::socket(m_io_service);
	
	std::error_code ec;

	m_sock.connect(ep, ec);

	if (ec.value())
		return false;

	return true;
}

size_t connection::read(std::string& buffer)
{
	char recv_data[BLOCK_SIZE];
	buffer.clear();

	size_t readed = 0;

	while (true)
	{
		size_t len = 0;

		std::error_code ec;
		len = m_sock.receive(asio::buffer(recv_data), 0, ec);

		if (ec.value())
			return 0;

		readed += len;

		if (len)
			buffer.append(recv_data, len);

		if (!buffer.empty())
			if (buffer.back() == '\n')
				return readed;
	}

	return readed;
}

size_t connection::write(const std::string& str)
{
	std::error_code ec;

	size_t len = m_sock.send(asio::buffer(str), 0, ec);
	
	if (ec.value())
		len = 0;

	return len;
}

void connection::disconnect()
{
	if (m_sock.is_open())
	{
		std::error_code ec = asio::error::connection_aborted;
		m_sock.close(ec);
		m_sock.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
	}
}
