#include <iostream>
#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <vector>
#include <string>
#include <sstream>
#include <ctime>


#define bufferSize 1024

using namespace std;

struct mailContent
{
    string from;
    string to;
    string cc;
    string bcc;
    string subject;
    string content;
    // Them file
};

int choiceUI()
{
    int choice = 0;
    cout << "Vui long chon Menu:\n";
    cout << "1. Gui mail\n";
    cout << "2. Xem cac mail da nhan\n";
    cout << "0. Thoat\n";
    cout << "Lua chon: ";
    cin >> choice;
    cout << "Ban chon: " << choice << endl;
    return choice;
}

bool serverReply(int client_fd)
{
    char buffer[1024];
    int valRead;

    if ((valRead = recv(client_fd, buffer, sizeof(buffer), 0)) < 0)
    {
        cout << "Fail to recv from server\n";
        return 0;
    }
    else
    {
        cout << "Server reply: ";
        for (int i = 0; i < valRead; i++)
        {
            cout << buffer[i];
        }
        cout << endl;
    }
    memset(buffer, 0, sizeof(buffer));
    return 1;
}

string getString()
{
    string a;
    int c;
    do 
    {
        c = getchar();
        if (c != '\n')
            a.push_back(c);
        else
            break;
    }
    while(1);
    return a;
}

mailContent writeMail()
{
    mailContent a;
    a.from = "khoi@hcmus.vn"; // Doc file config
    cout << "Day la thong tin soan mail (nhan Enter neu khong muon nhap gi hoac muon ket thuc viec nhap)\n";
    cin.ignore();
    cout << "To: ";
    a.to = getString();
    cout << "CC: ";
    a.cc = getString();
    cout << "BCC: ";
    a.bcc = getString();
    cout << "Subject: ";
    a.subject = getString();
    cout << "Content: ";
    a.content = getString();
    // Gui kem file

    return a;
}

bool sendInitData(mailContent a, int client_fd)
{
    bool status;
    string sendMsg;

    sendMsg = "MAIL FROM: <" + a.from + ">\r\n";
    send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);
    status = serverReply(client_fd);
    if (status == 0)
        return 0;

    sendMsg = "RCPT TO: <" + a.to + ">\r\n";
    send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);
    status = serverReply(client_fd);
    if (status == 0)
        return 0;

    return 1;
}

string getTime()
{
    time_t current_time = time(nullptr);
    struct tm *local_time = localtime(&current_time);
    string a = asctime(local_time);
    return a;
}

bool sendData(mailContent a, int client_fd)
{
    bool status;

    string sendMsg = "DATA\r\n";
    send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);
    status = serverReply(client_fd);
    if (status == 0)
        return 0;
    
    string dataa = "";
    dataa += "From: <" + a.from + ">\r\n";
    dataa += "To: <" + a.to + ">\r\n";
    dataa += "Subject: " + a.subject + "\r\n";
    dataa += "Date: " + getTime() + "\r\n";
    // MIME version
    dataa += "Content-Type: multipart/alternative;\r\n";
    dataa += "Content-Language: en-us\r\n\r\n";
    dataa += "Content-Type: text/plain;\r\n";
    // dataa += "charset="us-ascii""
    dataa += "Content-Transfer-Encoding: 7bit\r\n\r\n";

    dataa += a.content + "\r\n\r\n" + ".\r\n";

    sendMsg = dataa;
    send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);
    status = serverReply(client_fd);
    if (status == 0)
        return 0;

    return 1;
}

void smtp()
{
    int client_fd;
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        cout << "Create socket failed \n";
        return;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(2225);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    bool status = 1;

    // =======================================
    if (connect(client_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        cout << "Connection failed \n";
        close(client_fd);
        return;
    }
    else
    {
        status = serverReply(client_fd);
        if (status == 0)
            return;
    }

    // =======================================
    // Send hello
    string sendMsg = "EHLO Khoi\r\n";
    send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);

    status = serverReply(client_fd);
    if (status == 0)
        return;

    // =======================================
    // Enter mail
    mailContent a = writeMail();
    status = sendInitData(a, client_fd);
    if (status == 0)
        return;
    // Send Data
    cout << sendData(a, client_fd) << endl;

    // =======================================
    // Quit
    sendMsg = "QUIT\r\n";
    send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);
    status = serverReply(client_fd);
    if (status == 0)
        return;

    close(client_fd);
}

void pop3()
{
    return;
}

int main()
{
    // Load file config

    // UI
    int choice = choiceUI();
    cout << "======================\n";

    if (choice == 1)
    {
        smtp();
        cout << "======================\n";
    }
    else if (choice == 2)
    {
        pop3();
    }
    else
    {
        return 0;
    }
}