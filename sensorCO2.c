#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <ws2tcpip.h>
#pragma comment (lib, "ws2_32.lib")
#define TRUE 1
#define FALSE 0
// Configuracoes do socket
#define PORT 7640
#define MSG_LENGTH 64
#define ID_SEN_CO2 2

int main(int argc, char const *argv[]) {
  // Variaveis utilizadas na criacao e configuracao do cliente.
  int sensorSocket;  // Socket do sensor.
  int valRead;  // Valor de retorno durante a leitura de uma nova mensagem.
  char buffer[MSG_LENGTH] = {0};  // Buffer para recebimento e envio de dados.
  struct sockaddr_in serverAddr;  // Estrutura para as configuracoes de conexao com o servidor.
  WSADATA _data;  // Usada no carregamento da DLL do winsock.
  double value;

  // Carregando a DLL do winsock.
  if (WSAStartup(MAKEWORD(2,0), &_data) == SOCKET_ERROR) {
    fprintf(stderr, "Erro: Winsock DLL nao foi encontrada.\n");
    exit(EXIT_FAILURE);
  }
  // Criando o descritor do socket.
  if ((sensorSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == 0){
    fprintf(stderr, "Erro na criacao do socket.\n");
    exit(EXIT_FAILURE);
  }
  printf("Conectando com o servidor...\n");
  // Configurando com quem o socket ira se conectar.
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(PORT);
  // Ira se conectar com o localhost para teste.
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  // Tentativa de conexao com o servidor.
  if (connect(sensorSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
    fprintf(stderr, "Erro durante a conexao com o servidor.\n");
    exit(EXIT_FAILURE);
  }
  // Etapa de identificacao.
  snprintf(buffer, MSG_LENGTH, "%d", ID_SEN_CO2);
  send(sensorSocket, buffer, strlen(buffer), 0);
  memset(buffer, 0, MSG_LENGTH * sizeof(char));
  valRead = recv(sensorSocket, buffer, MSG_LENGTH, 0);
  if (strcmp(buffer, "0") != 0) {
    fprintf(stderr, "Erro no handshake com o servidor.\n");
    closesocket(sensorSocket);
    WSACleanup();
    exit(EXIT_FAILURE);
  }
  printf("Conectado.\n");
  // Envia dados periodicamente ao sevidor acerca da leitura do sensor.
  value = 550.0;
  while (TRUE) {
    snprintf(buffer, MSG_LENGTH, "%.2lf", value);
    send(sensorSocket, buffer, strlen(buffer), 0);
    memset(buffer, 0, MSG_LENGTH * sizeof(char));
    //value = value + ((rand() % 200) - 100) / 10.0;  // Variacao "realista"
    value = value + ((rand() % 200) - 100) / 1.0;  // Variacao para teste
    Sleep(1000);
  }
  // Fechando o socket.
  closesocket(sensorSocket);
  WSACleanup();
  return 0;
}
