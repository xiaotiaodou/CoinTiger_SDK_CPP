#pragma once
//  
//  NetworkRequest.cpp  
//  

#include "NetworkRequest.h"  

using namespace std;
using namespace boost;
using boost::asio::ip::tcp;

// POST����  
CString PostRequest(CString CShost, CString path, CString form)
{
	char *host = CShost.GetBuffer(CShost.GetLength());
	long length = form.GetLength();
	//form.Replace("\t", "");
	// ����Asio����: io_service��������Ȼ���  
	boost::asio::io_service io_service;

	// ��ȡ�������ն��б�  
	tcp::resolver resolver(io_service);
	tcp::resolver::query query(host, "http");
	tcp::resolver::iterator iter = resolver.resolve(query);

	// ��������ÿһ���նˣ�ֱ���ɹ�����socket����  
	tcp::socket socket(io_service);
 	try
 	{
		boost::asio::connect(socket, iter);
//		cout << "boost::asio::connect(socket, iter);" << endl;
 	}
 	catch (boost::system::system_error ec)
 	{
		cout << ec.what()<<endl;
		//throw boost::system::system_error(ec);
		//cout << "connect_error:" << ec <<endl;
 		return "connect_error";
 	}

	// ������������ͷ  
	// ָ�� "Connection: close" �ڻ�ȡӦ���Ͽ����ӣ�ȷ�����ļ�ȫ�����ݡ�  
	boost::asio::streambuf request;
	ostream request_stream(&request);
	request_stream << "POST " << path << " HTTP/1.1\r\n";
	request_stream << "Host: " << host << "\r\n";
	request_stream << "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/39.0.2171.71 Safari/537.36\r\n";
	//request_stream << "Accept: */*\r\n";
	request_stream << "Content-Type:application/json\r\n";
	request_stream << "Content-Length: " << length << "\r\n";
	request_stream << "Connection: close\r\n\r\n"; // ע����������������  
	request_stream << form; //POST ���͵����ݱ����������ڿ���  

							// ��������  
	boost::asio::write(socket, request);

	//cout << "boost::asio::write(socket, request);" << endl;
	// ��ȡӦ��״̬. Ӧ�𻺳��� streambuf ���Զ���������������  
	// �����������ڹ��컺����ʱͨ���������ֵ����  
	boost::asio::streambuf response;
	try
	{
		boost::asio::read_until(socket, response, "\r\n");
	}
	catch (...)
	{
			//cout << ec.what() << endl;
			//throw boost::system::system_error(ec);
			//cout << "connect_error:" << ec <<endl;
			return "connect_error";
	}
	//cout << "boost::asio::read_until(socket, response, \"\\r\\n\");" << endl;
	// ���Ӧ���Ƿ�OK.  
	istream response_stream(&response);// Ӧ����  
	string http_version;
	response_stream >> http_version;
	unsigned int status_code;
	response_stream >> status_code;
	string status_message;
	getline(response_stream, status_message);
	if (!response_stream || http_version.substr(0, 5) != "HTTP/")
	{
		printf(" error ��Ч��Ӧ\n");
	}
	if (status_code != 200)
	{
		printf(" error ��Ӧ���� status code %d\n", status_code);
	}
	//cout << "getline(response_stream, status_message);" << endl;

	// ��ȡӦ��ͷ�����������к�ֹͣ  
	boost::asio::read_until(socket, response, "\r\n\r\n");

	//cout << "boost::asio::read_until" << endl;
	// ��ʾӦ��ͷ��  
	string header;
	int len = 0;
	while (getline(response_stream, header) && header != "\r")
	{
		if (header.find("Content-Length: ") == 0) {
			stringstream stream;
			stream << header.substr(16);
			stream >> len;
		}
	}

	long size = response.size();

	if (size > 0) {
		// .... do nothing  
	}

	// ѭ����ȡ��������ֱ���ļ�����  
	boost::system::error_code error;
	while (boost::asio::read(socket, response, boost::asio::transfer_at_least(1), error))
	{
		// ��ȡӦ�𳤶�  
		size = response.size();
		if (len != 0) {
			//cout << size << "  Byte  " << (size * 100) / len << "%\n";
		}

	}

	//cout << "boost::asio::read" << endl;
	if (error != boost::asio::error::eof)
	{
		throw boost::system::system_error(error);
	}

	//cout << size << " Byte �������������." << endl;

	// ��streambuf����ת��ΪCString���ͷ���  
	istream is(&response);
	is.unsetf(ios_base::skipws);
	string sz;
	sz.append(istream_iterator<char>(is), istream_iterator<char>());
	//cout << "���ͽ���;" << endl;

	// ����ת������ַ���  
	return sz.c_str();
}



CString GetRequest(CString sendmsg)
{
	HINTERNET internetopen;
	HINTERNET internetopenurl;
	BOOL internetreadfile;
	DWORD byteread = 0;
	char buffer[1];
	//char ch;
	memset(buffer, 0, 1);
	internetopen = InternetOpen(_T("Testing"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (internetopen == NULL) {
		//cout << "InternetOpen��ʼ��ʧ��!" << endl;
		return "Can't InternetOpen!";
	}
	internetopenurl = InternetOpenUrl(internetopen, sendmsg, NULL, 0, INTERNET_FLAG_RELOAD, 0);
	if (internetopenurl == NULL) {
		//cout << "InternetOpenUrl��Urlʧ��!" << endl;
		InternetCloseHandle(internetopen);
		return "Can't InternetOpenUrl!";
	}
	CString buffs = "";
	while (1) {
		internetreadfile = InternetReadFile(internetopenurl, buffer, sizeof(char), &byteread);
		if (byteread == 0) {
			InternetCloseHandle(internetopenurl);
			break;
		}
		buffs += buffer[0];
	}
	if (internetreadfile == FALSE)
	{
		InternetCloseHandle(internetopenurl);
		return "Can't InternetReadFile!";
	}
	return buffs;
}
