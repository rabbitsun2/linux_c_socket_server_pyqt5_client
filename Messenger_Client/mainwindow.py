# Subject: PyQt5 폼 to 폼 열기 예제
# Author: 정도윤(Doyun Jung)
# Created Date: 2022-04-07
# Description:

import os
import sys
import socket
import threading

from PyQt5.QtWidgets import *
from PyQt5.QtCore import QCoreApplication
from PyQt5 import uic

form_class = uic.loadUiType("form.ui")[0]

class MyWindow(QMainWindow, form_class):

    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.initUI()

    def initUI(self):
        print("Hello")

        # 초기 계정값
        self.line_email.setText("rabbit.white@daum.net")
        self.line_passwd.setText("a1234b")
        self.line_passwd.setEchoMode(QLineEdit.Password)

        self.btn_second.clicked.connect(self.action_clicked_btn_second)
        self.btn_disconnect.clicked.connect(self.action_clicked_btn_disconnect)
        self.btn_testConnect.clicked.connect(self.action_clicked_btn_test_connection)
        self.btn_connect.clicked.connect(self.action_clicked_btn_connect)

        #self.line_ipaddr.setText("10.210.150.60")
        self.line_ipaddr.setText("112.76.56.88")
        self.line_port.setText("8888")

    def action_clicked_btn_second(self):
        print("버튼 클릭")
        self.plainMessage.setPlainText("String")
        self.startTimer()

    def action_clicked_btn_test_connection(self):
        self.sendRecv("test_alert")

    def action_clicked_btn_connect(self):

        strEmail = "id," + self.line_email.text() + "#"
        strPasswd = "passwd," + self.line_passwd.text() + "#"

        self.stack = []
        self.stack.append(strEmail)
        self.stack.append(strPasswd)

        # 읽기 전용
        self.line_ipaddr.setReadOnly(1)
        self.line_port.setReadOnly(1)

        # 교착 상태
        self.locker = 0
        self.activeConnection()

    def action_clicked_btn_disconnect(self):
        print("Timer를 종료합니다")
        timer = self.timer
        print(type(timer))

        # 읽기, 쓰기
        self.line_ipaddr.setReadOnly(0)
        self.line_port.setReadOnly(0)

        if self.locker == 0:
            self.locker = 1
            timer.join()
            timer.cancel()


    def startTimer(self):
        print("Timer")
        self.timer = threading.Timer(5, self.startTimer)
        self.timer.start()


    def activeConnection(self):
        self.timer = threading.Timer(5, self.activeConnection)
        message = ""

        print(len(self.stack))

        if len(self.stack) > 0:
            message = self.stack.pop()
            self.sendRecv(message)

        # 교착 상태 방지
        if self.locker == 0:
            self.timer.start()


    def sendRecv(self, message):
        HOST = self.line_ipaddr.text()
        PORT = int(self.line_port.text())

        #strMessage = ""

        # 소켓 객체를 생성합니다.
        # 주소 체계(address family)로 IPv4, 소켓 타입으로 TCP 사용합니다.
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        # 지정한 HOST와 PORT를 사용하여 서버에 접속합니다.
        client_socket.connect((HOST, PORT))

        # 메시지를 전송합니다.
        # client_socket.sendall('id,rabbit.white@daum.net,'.encode())
        client_socket.sendall(message.encode())
        client_socket.sendall('참'.encode())

        # 메시지를 수신합니다.
        data = client_socket.recv(1024)
        print('Received', repr(data.decode()))

        strMessage = repr(data.decode())

        # 소켓을 닫습니다.
        client_socket.close()

        # 태스트 연결 Q메시지박스 출력
        if message == "test_alert":
            reply = QMessageBox.information(self, 'Message', strMessage)

        #reply = QMessageBox.information(self, 'Message', strMessage)


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MyWindow()
    window.show()
    app.exec_()
