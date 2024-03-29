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
#include <fstream>
#include <mimetic/mimetic.h>
#include <mimetic/mimeentity.h>


#define bufferSize 1024

using namespace std;
using namespace mimetic;

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

struct config
{
    string uname;
    string pass;
    string mailServer;
    int smtp;
    int pop3;
    int autoLoad;
};

config readConfig()
{
    config con;
    fstream file;
    file.open("config.txt", ios::in);
    getline(file, con.uname);
    getline(file, con.pass);
    getline(file, con.mailServer);
    string tmp;
    getline(file, tmp);
    con.smtp = stoi(tmp);
    getline(file, tmp);
    con.pop3 = stoi(tmp);
    getline(file, tmp);
    con.autoLoad = stoi(tmp);
    file.close();
    return con;
}

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

mailContent writeMail(config con)
{
    mailContent a;
    a.from = con.uname; // Doc file config
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
    string fileChoice;
    // cin.ignore();
    cout << "Ban co muon dinh kem file khong (1. Co | 0. Khong): \n";
    // getline(cin, fileChoice);
    // cin.ignore();
    cin >> fileChoice;
    cin.ignore();
    if (fileChoice == "1")
    {
        string fileName;
        cout << "Nhap ten file: ";
        fileName = getString();
        // cin.ignore();
        string filePath;
        cout << "Nhap duong dan cua file: ";
        filePath = getString();
        // cin.ignore();

        string fileData;
        fstream file;
        file.open(filePath, ios::in);
        while (!file.eof())
        {
            string tmp;
            getline(file, tmp);
            fileData += tmp + '\n';
        }
        file.close();
        cout << "Data: \n";
        cout << fileData << endl;
    }
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

// bool sendDataWithMIME(mailContent a, int client_fd)
// {
//     bool status;

//     string sendMsg = "DATA\r\n";
//     send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);
//     status = serverReply(client_fd);
//     if (status == 0)
//         return 0;
    
//     MimeEntity me;

//     cout << "Im here\n";
//     me.header().from(a.from);
//     me.header().to(a.to);
//     me.header().cc(a.cc);
//     me.header().bcc(a.bcc);
//     me.header().subject(a.subject);

//     me.body().assign(a.content);

//     cout << me << endl;

//     return 1;
// }

void smtp(config con)
{
    int client_fd;
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        cout << "Create socket failed \n";
        return;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(con.smtp);
    serv_addr.sin_addr.s_addr = inet_addr(con.mailServer.c_str());
    cout << con.mailServer.c_str() << endl;

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
    mailContent a = writeMail(con);
    status = sendInitData(a, client_fd);
    if (status == 0)
        return;
    // Send Data
    cout << sendData(a, client_fd) << endl;
    // cout << sendDataWithMIME(a, client_fd) << endl;

    // =======================================
    // Quit
    sendMsg = "QUIT\r\n";
    send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);
    status = serverReply(client_fd);
    if (status == 0)
        return;

    close(client_fd);
}

vector<string> readDataPOP3(string a, int valRead)
{
    vector<string> parts;
    int start = 0;
    while (1)
    {
        int pos = a.find("\r\n\r\n", start);
        string tmp1 = a.substr(start, pos - start);
        // cout << tmp1 << endl << "==============\n";
        if (tmp1 == ".\r\n")
            break;
        parts.push_back(tmp1);
        // string tmp = a.substr(pos - start + 4);
        start = pos + 4;
    }
    return parts;
}

void pop3()
{
    int client_fd;
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        cout << "Create socket failed \n";
        return;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(3335);
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
    // Send CAPA
    string sendMsg = "CAPA\r\n";
    send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);

    status = serverReply(client_fd);
    if (status == 0)
        return;

    // =======================================
    // Send User info
    cout << "Going here\n";
    sendMsg = "USER khoi2@hcmus.vn\r\n"; // Read from configure file
    send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);
    cout << "End here\n";
    status = serverReply(client_fd);
    if (status == 0)
        return;

    // =======================================
    // Send User password
    sendMsg = "PASS 222\r\n"; // Read from configure file
    send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);

    status = serverReply(client_fd);
    if (status == 0)
        return;

    // =======================================
    // Send STAT and LIST; DOC CHO NAY CAN THAN
    sendMsg = "STAT\r\n"; // Read from config file
    send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);

    status = serverReply(client_fd);
    if (status == 0)
        return;

    sendMsg = "LIST\r\n"; // Read from configure file
    send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);

    status = serverReply(client_fd);
    if (status == 0)
        return;

    // =======================================
    // Send UIDL
    sendMsg = "UIDL\r\n"; // Read from configure file
    send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);

    status = serverReply(client_fd);
    if (status == 0)
        return;

    // =======================================
    // Send Request to get mail
    sendMsg = "RETR 1\r\n"; // Read from configure file
    send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);

    // =======================================
    // Receive new mail
    char buffer[bufferSize * 5];
    // string buffer;
    int valRead;

    if ((valRead = recv(client_fd, buffer, sizeof(buffer), 0)) < 0)
    {
        cout << "Fail to recv from server\n";
        return;
    }
    else
    {
        vector<string> parts = readDataPOP3(buffer, valRead);
        for (int i = 0; i < parts.size(); i++)
        {
            cout << parts[i] << endl;
            cout << "++++++++++++++\n";
        }
    }

}

int main()
{
    // Load file config
    config con;
    con = readConfig();

    // UI
    int choice = 0;
    do
    {
        choice = choiceUI();
        cout << "======================\n";

        if (choice == 1)
        {
            smtp(con);
            cout << "======================\n";
        }
        else if (choice == 2)
        {
            pop3();
            cout << "======================\n";
        }
    }
    while (choice != 0);
}