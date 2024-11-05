# Project: Email Client

This project requires students to build an **Email Client** program with the primary goal of getting familiar with socket programming and understanding the **SMTP** and **POP3** protocols in data exchange between client and server.

## General Guidelines
- **Programming Environment**: Unix/Linux.
- **Programming Language**: C/C++.
- **Socket Library**: Only the `socket` library is allowed. Using third-party libraries for SMTP or POP3 is prohibited.

## Evaluation Criteria
**Program:**
- The project focuses on socket programming and understanding the SMTP and POP3 protocols between client and server.
- Evaluation is based on the main functionalities listed in the project requirements.

## Project Requirements
### Key Features of the Email Client:
1. **Send email** (SMTP protocol): Supports To, CC, BCC fields and file attachments. Recipients in To and CC will not see BCC recipients.
2. **Download email** (POP3 protocol): Fetches emails accurately from the Mail Server, avoids re-downloading emails, even after restarting the Mail Client.
3. **Email categorization**: Organizes emails into specific folders based on filters defined in a configuration file.
4. **Spam filtering**: Moves spam emails based on keywords in subject or content to the Spam folder.
5. **Email status management**: Tracks read/unread status of emails, and maintains this status even when the Mail Client restarts.
6. **Optional**: Automatically fetches emails every 10 seconds (adjustable in the config file).

### Example Configuration File
```plaintext
Filter:
// Move emails from ahihi@testing.com to the Project folder
From: ahihi@testing.com - To folder: Project
// Move emails with "urgent" or "ASAP" in the subject to Important folder
Subject: "urgent", "ASAP" - To folder: Important
// Move emails with "report" or "meeting" in the content to Work folder
Content: "report", "meeting" - To folder: Work
// Move emails with "virus", "hack", or "crack" in subject or content to Spam
Spam: "virus", "hack", "crack" - To folder: Spam
