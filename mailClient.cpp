#include <iostream>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <vector>
#include <string>
#include <sstream>
#include <ctime>
#include <fstream>
// #include <mimetic/mimetic.h>
// #include <mimetic/mimeentity.h>
// #include <mimetic/utils.h>
#include <filesystem>


#define bufferSize 1024

using namespace std;
using namespace filesystem;
// using namespace mimetic;

struct mailContent
{
    string from;
    string to;
    string cc;
    string bcc;
    string subject;
    string content;
    // Them file
    string fileName;
    string filePath;
    string fileData;
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

string serverReplyStr(int client_fd)
{
    char buffer[1024];
    int valRead;
    string rep = "";

    if ((valRead = recv(client_fd, buffer, sizeof(buffer), 0)) < 0)
    {
        cout << "Fail to recv from server\n";
    }
    else
    {
        cout << "Server reply: ";
        for (int i = 0; i < valRead; i++)
        {
            // cout << buffer[i];
            rep += buffer[i];
        }
        // cout << endl;
    }
    memset(buffer, 0, sizeof(buffer));
    return rep;
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

int getFileSize(string filePath)
{
    int size;
    fstream file;
    file.open(filePath, ios::in | ios::binary | ios::ate);
    size = static_cast<int>(file.tellg());
    file.close();
    return size;
}

mailContent writeMail(config con)
{
    mailContent a;
    a.from = con.uname; // Doc file config
    // cin.ignore();
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
        do
        {
            // string fileName;
            cout << "Nhap ten file: ";
            a.fileName = getString();
            // cin.ignore();
            cout << "Nhap duong dan cua file: ";
            a.filePath = getString();
            // cin.ignore();

            if (getFileSize(a.filePath) > 3000000)
            {
                cout << "File > 3MB. Hay chon file khac\n";
            }
            else
                break;
        } while (1);
        
        // Check file size
        fstream file;
        file.open(a.filePath, ios::in);
        while (!file.eof())
        {
            string tmp;
            getline(file, tmp);
            a.fileData += tmp + '\n';
        }
        a.fileData += "-----";
        file.close();
        // cout << "Data: \n";
        // cout << a.fileData << endl;
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

    if (a.cc != "")
    {
        sendMsg = "RCPT TO: <" + a.cc + ">\r\n";
        send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);
        status = serverReply(client_fd);
        if (status == 0)
            return 0;
    }

    if (a.bcc != "")
    {
        sendMsg = "RCPT TO: <" + a.bcc + ">\r\n";
        send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);
        status = serverReply(client_fd);
        if (status == 0)
            return 0;
    }

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
    dataa += "Date: " + getTime();
    dataa += "From: " + a.from + "\r\n";
    dataa += "To: <" + a.to + ">\r\n";
    if (a.cc != "")
        dataa += "CC: <" + a.cc + ">\r\n";
    if (a.bcc != "")
        dataa += "BCC: <" + a.bcc + ">\r\n";
    dataa += "Subject: " + a.subject + "\r\n\r\n";

    // Content part
    // dataa += "Content-Type: multipart/alternative;\r\n";
    // dataa += "Content-Language: en-us\r\n\r\n";
    // dataa += "Content-Type: text/plain;\r\n";
    // dataa += "charset="us-ascii""
    // dataa += "Content-Transfer-Encoding: 7bit\r\n\r\n";

    dataa += a.content + "\r\n\r\n";

    // File attachment part
    if (a.fileName != "")
    {
        // dataa += "Content-Type: text/plain; charset: UTF-8; name=" + a.fileName + "\r\n";
        // dataa += "Content-Disposition: attachment; filename=" + a.fileName + "\r\n";
        // dataa += "Content-Transfer-Encoding: 7bit\r\n\r\n";

        dataa += a.fileName + "\r\n";
        dataa += a.fileData + "\r\n\r\n";
    }


    dataa += ".\r\n";

    sendMsg = dataa;
    send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);
    status = serverReply(client_fd);
    if (status == 0)
        return 0;

    return 1;
}

// class MyMimeEntity : public MimeEntity {
// public:
//     string toMimeString() {
//         stringstream ss;
//         this->write(ss);
//         return ss.str();
//     }
// };

// MyMimeEntity writeContent(string content)
// {
//     MyMimeEntity me;
//     // me.header().;
//     me.header().contentType().set("text");
//     me.header().contentType().subtype("plain");
//     me.header().contentType().param("charset", "UTF-8");
//     me.header().contentType().param("format", "flowed");
//     me.header().contentTransferEncoding("7bit");
//     me.body().assign(content);
//     return me;
// }

// MyMimeEntity attachFile(string fileName, string filePath)
// {
//     MyMimeEntity me;

//     fstream file;
//     file.open(filePath, ios::in);
//     if (!file)
//     {
//         cout << "Error to open file\n";
//         file.close();
//         return me;
//     }


//     stringstream ss;
//     ss << file.rdbuf();
//     // me.body().assign("-----");
//     me.body().assign(ss.str());
//     me.header().contentType().set("text");
//     me.header().contentType().subtype("plain");
//     me.header().contentType().param("name", fileName);
//     me.header().contentType().param("charset", "us-ascii");
//     me.header().contentDisposition("attachment");
//     me.header().contentDisposition().param("filename", fileName);
//     file.close();
//     return me;
// }

// bool sendDataWithMIME(mailContent a, int client_fd)
// {
//     bool status;

//     string sendMsg = "DATA\r\n";
//     send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);
//     status = serverReply(client_fd);
//     if (status == 0)
//         return 0;
    
//     MyMimeEntity me;
//     cout << "Im here\n";
//     me.header().contentType().set("multipart");
//     me.header().contentType().subtype("mixed");
//     me.header().contentType().param("boundary", "-----");
//     me.header().from(a.from);
//     me.header().to(a.to);
//     me.header().cc(a.cc);
//     me.header().bcc(a.bcc);
//     me.header().subject(a.subject);

//     // Write content
//     MyMimeEntity me1 = writeContent(a.content);
//     // me.body().append(&me2);
    
//     // // Attach file
//     // MyMimeEntity me2;
//     // if (a.fileName != "")
//     // {
//     //     me2 = attachFile(a.fileName, a.filePath);
//     // }

//     // stringstream ss;
//     // // ss << me.header();
//     sendMsg = me.toMimeString() + "\r\n\r\n";
//     sendMsg += "-----\r\n";
//     sendMsg += me1.toMimeString() + "\r\n\r\n";
//     // sendMsg += me2.toMimeString() + "\r\n\r\n";
//     sendMsg += "-----\r\n.\r\n";
//     send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);

//     cout << sendMsg << endl;

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
    // sendDataWithMIME(a, client_fd);

    // =======================================
    // Quit
    sendMsg = "QUIT\r\n";
    send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);
    status = serverReply(client_fd);
    if (status == 0)
        return;

    close(client_fd);
}

// =================================

struct listMail
{
    string stt;
    int bytes;
};

vector<listMail> readLIST(string a)
{
    vector<listMail> li;
    stringstream ss(a);
    string tmp;
    getline(ss, tmp);
    // cout << tmp << endl;
    while(1)
    {
        listMail l;
        getline(ss, tmp);
        cout << tmp << endl;
        if (tmp == ".\r")
        {
            cout << "Vao r\n";
            break;
        }
        stringstream sss(tmp);
        getline(sss, tmp, ' ');
        l.stt = tmp;
        getline(sss, tmp);
        // cout << tmp << endl;
        l.bytes = stoi(tmp);
        li.push_back(l);
    }
    cout << "Ra roi\n";
    return li;
}

vector<string> readDataPOP3(string a)
{
    cout << a << endl;
    vector<string> parts;
    int start = 0;
    while (1)
    {
        int pos = a.find("\r\n\r\n", start);
        string tmp1 = a.substr(start, pos - start);
        cout << tmp1 << endl << "==============\n";
        if (tmp1[0] == '.')
            break;
        parts.push_back(tmp1);
        // string tmp = a.substr(pos - start + 4);
        start = pos + 4;
    }
    return parts;
}

struct mailParts
{
    string header;
    string content;
    string fileName;
    string filePath;
};

vector<string> readFileAttach(string a)
{
    vector<string> v;
    stringstream ss(a);
    string tmp;

    getline(ss, tmp);
    v.push_back(tmp);

    tmp = ss.str();
    v.push_back(tmp);
    return v;
}

void writeFileAttach(string fileName, string data, string mailAccount)
{
    fstream file;
    file.open(mailAccount + "/files/" + fileName, ios::out);
    file << data;
    file.close();
}

void writeMailToFolder(mailParts mp, string mailAccount, string idMail, string mailType)
{
    cout << mp.fileName << endl;
    fstream file;
    file.open(mailAccount + "/" + mailType + "/" + idMail + ".txt", ios::out);
    file << mp.header << "\r\n---\r\n" << mp.content << "\r\n";
    if (mp.fileName != "")
    {
        string tmp = " is attach";
        string tmp2 = mp.fileName.insert(mp.fileName.length() - 1, tmp);
        cout << "aaaaaaaaaaaaaaaaaaaaaaaa " << tmp2 << endl;
        mp.fileName = tmp2;
        file << "\r\n";
    }
    file << mp.fileName << "\r\n" << ".\r\n";
    file.close();
}

mailParts readDataPop3FromMail(vector<string> parts, string mailAccount)
{
    mailParts mp;

    mp.header = parts[0];
    mp.content = parts[1];

    if (parts.size() > 2)
    {
        cout << "HAHAHAHAHA " << parts[2] << endl;
        vector<string> v = readFileAttach(parts[2]);
        mp.fileName = v[0];
        writeFileAttach(v[0], v[1], mailAccount);
        mp.filePath = mailAccount + "/files/" + mp.fileName;
    }
    
    return mp;
}

void createMailBoxFolder(string mailAccount)
{
    if (create_directory(mailAccount) == 0)
        cout << "Thu muc da duoc tao\n";
    if (create_directory(mailAccount + "/inbox"))
        cout << "Tao thu muc inbox thanh cong\n";
    if (create_directory(mailAccount + "/files"))
        cout << "Tao thu muc files thanh cong\n";

}

struct mailStatus
{
    string id;
    string status;
};

vector<mailStatus> outputStatus(string mailAccount, string mailType)
{
    vector<mailStatus> ms;
    fstream file;
    file.open(mailAccount + "/" + mailType + "/" + "status.txt");
    if (!file.is_open())
    {
        cout << "No\n";
        return ms;
    }
    do
    {
        mailStatus mss;
        getline(file, mss.id);
        getline(file, mss.status);
        cout << mss.id << " --- " << mss.status << endl;
        if (mss.id != "")
            ms.push_back(mss);
    }
    while(!file.eof());
    file.close();
    return ms;
}

// void inputStatus()
// {

// }

vector<string> getMailData(string mailAccount, string type, string id)
{
    vector<string> p;
    cout << "Here\n";
    string dataa = "";
    fstream file;
    file.open(mailAccount + "/" + type + "/" + id + ".txt", ios::in);
    if (!file.is_open())
    {
        cout << "NO\n";
        return p;
    }
    while (!file.eof())
    {
        string tmp;
        getline(file, tmp);
        dataa += tmp + "\r\n";
    }
    file.close();
    // cout << dataa << endl;

    string tmp;
    int pos = dataa.find("\r\n");
    tmp = dataa.substr(pos + 2);
    // cout << "---\n";
    // cout << tmp << endl;

    int pos2 = tmp.find("---");
    p.push_back(tmp.substr(0, pos2));
    p.push_back(tmp);
    // cout << "+++++++++++=\n";
    // cout << p[0] << "\nhahaha\n" << p[1] << endl;
    return p;
}

vector<vector<string>> getAllMailData(vector<mailStatus> ms, string type, string mailAccount)
{
    vector<vector<string>> mp;
    for (int i = 0; i < ms.size(); i++)
    {
        vector<string> mps = getMailData(mailAccount, type, ms[i].id);
        mp.push_back(mps);
        continue;
    }
    return mp;
}

vector<vector<string>> printMailBox(vector<mailStatus> ms, string type, string mailAccount)
{
    vector<vector<string>> mp = getAllMailData(ms, type, mailAccount);
    for (int i = 0; i < mp.size(); i++)
    {
        cout << "---------------" << i + 1 << "---------------";
        if (ms[i].status == "0")
            cout << "(chua doc)";
        cout << endl;
        cout << mp[i][0] << endl;
        cout << "-------------------------------\n";
    }
    return mp;
}

void readMail(int choice, string dataa)
{
    cout << "\nNoi dung mail thu " << choice << endl;
    cout << "==============================\n";
    cout << dataa << endl;
    cout << "==============================\n";
}

int choiceMailToRead(string mailType)
{
    vector<mailStatus> ms = outputStatus("khoi3@hcmus.vn", "inbox");
    cout << "Day la danh sach mail trong folder Inbox\n";
    vector<vector<string>> mp = printMailBox(ms, mailType, "khoi3@hcmus.vn");

    int choice = 0;
    do
    {
        cout << "Hay chon mail muon doc (chon 0 neu muon huy): ";
        cin.ignore();
        cin >> choice;
        if (choice == 0)
            return 0;
        cout << "Ban chon thu: " << choice << endl;
    }
    while(choice > ms.size());
    readMail(choice, mp[choice - 1][1]);

    return 0;
}

// string getMailType(string header)
// {
//     cout << header << "---" << endl;
//     int pos = header.find("<");
//     int pos1 = header.find(">\r");
//     string mail = header.substr(pos + 1, pos1 - pos - 1);
//     return mail;
// }

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

    // Create mail box for user
    createMailBoxFolder("khoi3@hcmus.vn");

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
    // cout << "Going here\n";
    sendMsg = "USER khoi3@hcmus.vn\r\n"; // Read from configure file
    send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);
    // cout << "End here\n";
    status = serverReply(client_fd);
    if (status == 0)
        return;

    // =======================================
    // Send User password
    sendMsg = "PASS 333\r\n"; // Read from configure file
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

    string rep = serverReplyStr(client_fd);
    vector<listMail> li = readLIST(rep);

    // Phai di so voi cac mail da nhan roi moi chay RETR

    // =======================================
    // Send UIDL
    sendMsg = "UIDL\r\n"; // Read from configure file
    send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);

    status = serverReply(client_fd);
    if (status == 0)
        return;

    // =======================================
    // Send Request to get mail
    cout << "Mail thu: " << li[li.size() - 1].stt << endl;
    sendMsg = "RETR " + li[li.size() - 1].stt + "\r\n"; // Read from configure file
    send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);

    // =======================================
    // Receive new mail
    char buffer[bufferSize];
    // memset(buffer, 0, sizeof(buffer));
    int valRead;

    if ((valRead = recv(client_fd, buffer, bufferSize, 0)) < 0)
    {
        cout << "Fail to recv from server\n";
        return;
    }
    else
    {
        cout << "Start =============\n";

        vector<string> parts = readDataPOP3(buffer);
        cout << "Mid ===============\n";
        for (int i = 0; i < parts.size(); i++)
        {
            cout << parts[i] << endl;
            cout << "++++++++++++++\n";
        }

        cout << "Im here 1\n";
        mailParts mp = readDataPop3FromMail(parts, "khoi3@hcmus.vn");

        cout << getMailType(mp.header) << " bla\n";

        // cout << "Im here 2\n";
        // writeMailToFolder(mp, "khoi3@hcmus.vn", li[li.size() - 1].stt, "inbox");
        // cout << "Im here 3\n";
        // int c = choiceMailToRead("inbox");


        // ios_base::sync_with_stdio(false);        // optimization
        // istringstream iss(buffer);
        // istreambuf_iterator<char> bit(iss), eit; // get stdin iterators
        // MimeEntity me(bit, eit);                       // parse and load message
        // printMimeStructure(&me);                      // print msg structure
    }

    sendMsg = "QUIT\r\n"; // Read from configure file
    send(client_fd, sendMsg.c_str(), sendMsg.length(), 0);

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
        }
        else if (choice == 3)
        {
            // cout << getFileSize("./config.txt");
        }
    }
    while (choice != 0);
}