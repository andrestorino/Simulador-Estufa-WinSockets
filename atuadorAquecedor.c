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
#define ID_ATU_AQ 3

int main(int argc, char const *argv[]) {
  // Variaveis utilizadas na criacao e configuracao do cliente.
  int actuatorSocket;  // Socket do atuador.
  int valRead;  // Valor de retorno durante a leitura de uma nova mensagem.
  char buffer[MSG_LENGTH] = {0};  // Buffer para recebimento e envio de dados.
  struct sockaddr_in serverAddr;  // Estrutura para as configuracoes de conexao com o servidor.
  WSADATA _data;  // Usada no carregamento da DLL do winsock.
  //Indica se o atuador esta ligado ou desligado
  int isOn = FALSE;

  // Carregando a DLL do winsock.
  if (WSAStartup(MAKEWORD(2,0), &_data) == SOCKET_ERROR) {
    fprintf(stderr, "Erro: Winsock DLL nao foi encontrada.\n");
    exit(EXIT_FAILURE);
  }
  // Criando o descritor do socket.
  if ((actuatorSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == 0){
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
  if (connect(actuatorSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
    fprintf(stderr, "Erro durante a conexao com o servidor.\n");
    exit(EXIT_FAILURE);
  }
  // Etapa de identificacao.
  snprintf(buffer, MSG_LENGTH, "%d", ID_ATU_AQ);
  send(actuatorSocket, buffer, strlen(buffer), 0);
  memset(buffer, 0, MSG_LENGTH * sizeof(char));
  valRead = recv(actuatorSocket, buffer, MSG_LENGTH, 0);
  if (strcmp(buffer, "0") != 0) {
    fprintf(stderr, "Erro no handshake com o servidor.\n");
    closesocket(actuatorSocket);
    WSACleanup();
    exit(EXIT_FAILURE);
  }
  printf("Conectado.\n");
  // Espera o recebimento do sinal do servidor indicando se ele deve ligar ou desligar.
  while (TRUE) {
    memset(buffer, 0, MSG_LENGTH * sizeof(char));
    valRead = recv(actuatorSocket, buffer, MSG_LENGTH, 0);
    sscanf(buffer, "%d", &isOn);
    if (isOn) printf("Aquecimento ativado.\n");
    else printf("Aquecimento desativado.\n");
  }
  // Fechando o socket.
  closesocket(actuatorSocket);
  WSACleanup();
  return 0;
}
