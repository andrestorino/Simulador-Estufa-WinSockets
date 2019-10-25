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
#define ID_CLIENT 7
// Possiveis tipos de requisicoes
#define REQ_MIN_TEMP 1
#define REQ_MIN_UMI 2
#define REQ_MIN_CO2 3
#define REQ_MAX_TEMP 4
#define REQ_MAX_UMI 5
#define REQ_MAX_CO2 6
#define REQ_READING 7

int main(int argc, char const *argv[]) {
  // Variaveis utilizadas na criacao e configuracao do cliente.
  int clientSocket;  // Socket do cliente.
  int valRead;  // Valor de retorno durante a leitura de uma nova mensagem.
  char buffer[MSG_LENGTH] = {0};  // Buffer para recebimento e envio de dados.
  struct sockaddr_in serverAddr;  // Estrutura para as configuracoes de conexao com o servidor.
  WSADATA _data;  // Usada no carregamento da DLL do winsock.
  // Variaveis utilizadas na aplicacao
  int op;  // Indica a operacao que o usuario escolheu.
  double input;  // Valor de entrada para o usuario configurar a estufa.
  double temp;  // Dados recebidos sobre a temperatura da estufa.
  double hum;  // Dados recebidos sobre a umidade do solo da estufa.
  double co2;  // Dados recebidos sobre o nivel de CO2 da estufa.

  // Carregando a DLL do winsock.
  if (WSAStartup(MAKEWORD(2,0), &_data) == SOCKET_ERROR) {
    fprintf(stderr, "Erro: Winsock DLL nao foi encontrada.\n");
    exit(EXIT_FAILURE);
  }
  // Criando o descritor do socket.
  if ((clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == 0){
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
  if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
    fprintf(stderr, "Erro durante a conexao com o servidor.\n");
    exit(EXIT_FAILURE);
  }
  // Etapa de identificacao.
  snprintf(buffer, MSG_LENGTH, "%d", ID_CLIENT);
  send(clientSocket, buffer, strlen(buffer), 0);
  memset(buffer, 0, MSG_LENGTH * sizeof(char));
  valRead = recv(clientSocket, buffer, MSG_LENGTH, 0);
  if (strcmp(buffer, "0") != 0) {
    fprintf(stderr, "Erro no handshake com o servidor.\n");
    closesocket(clientSocket);
    WSACleanup();
    exit(EXIT_FAILURE);
  }
  printf("Conectado.\n");
  // Envia dados periodicamente ao sevidor acerca da leitura do sensor.
  op = 8;
  while (op != 0) {
    memset(buffer, 0, MSG_LENGTH * sizeof(char));
    printf("Digite o tipo de requisicao (ou 9 para ajuda, ou 0 para sair): ");
    scanf("%d", &op);
    switch (op) {
      case 0:
        printf("Finalizando aplicacao...\n");
        break;
      case 1:
        printf("Digite o novo valor minimo de temperatura da estufa (9999 para voltar): ");
        scanf("%lf", &input);
        printf("Atualizando configuracao...\n");
        snprintf(buffer, MSG_LENGTH, "%d %lf", REQ_MIN_TEMP, input);
        send(clientSocket, buffer, strlen(buffer), 0);
        printf("Configuracao atualizada.\n\n");
        break;
      case 2:
        printf("Digite o novo valor minimo da umidade do solo da estufa (9999 para voltar): ");
        scanf("%lf", &input);
        printf("Atualizando configuracao...\n");
        snprintf(buffer, MSG_LENGTH, "%d %lf", REQ_MIN_UMI, input);
        send(clientSocket, buffer, strlen(buffer), 0);
        printf("Configuracao atualizada.\n\n");
        break;
      case 3:
        printf("Digite o novo valor minimo do nivel de CO2 da estufa (9999 para voltar): ");
        scanf("%lf", &input);
        printf("Atualizando configuracao...\n");
        snprintf(buffer, MSG_LENGTH, "%d %lf", REQ_MIN_CO2, input);
        send(clientSocket, buffer, strlen(buffer), 0);
        printf("Configuracao atualizada.\n\n");
        break;
      case 4:
        printf("Digite o novo valor maximo de temperatura da estufa [Celsius] (9999 para voltar): ");
        scanf("%lf", &input);
        printf("Atualizando configuracao...\n");
        snprintf(buffer, MSG_LENGTH, "%d %lf", REQ_MAX_TEMP, input);
        send(clientSocket, buffer, strlen(buffer), 0);
        printf("Configuracao atualizada.\n\n");
        break;
      case 5:
        printf("Digite o novo valor maximo da umidade do solo da estufa [%] (9999 para voltar): ");
        scanf("%lf", &input);
        if (input < 0.0 || input > 1.0) printf("Valor invalido [0.0 - 1.0]\n\n");
        else {
          printf("Atualizando configuracao...\n");
          snprintf(buffer, MSG_LENGTH, "%d %lf", REQ_MAX_UMI, input);
          send(clientSocket, buffer, strlen(buffer), 0);
          printf("Configuracao atualizada.\n\n");
        }
        break;
      case 6:
        printf("Digite o novo valor maximo do nivel de CO2 da estufa [ppm] (9999 para voltar): ");
        scanf("%lf", &input);
        printf("Atualizando configuracao...\n");
        snprintf(buffer, MSG_LENGTH, "%d %lf", REQ_MAX_CO2, input);
        send(clientSocket, buffer, strlen(buffer), 0);
        printf("Configuracao atualizada.\n\n");
        break;
      case 7:
        printf("Obtendo dados requisitados...\n");
        snprintf(buffer, MSG_LENGTH, "%d", REQ_READING);
        send(clientSocket, buffer, strlen(buffer), 0);
        memset(buffer, 0, MSG_LENGTH * sizeof(char));
        valRead = recv(clientSocket, buffer, MSG_LENGTH, 0);
        printf("Dados obtidos.\n");
        sscanf(buffer, "%lf %lf %lf", &temp, &hum, &co2);
        printf("\tTemperatura: %.2lf\n", temp);
        printf("\tUmidade do solo: %.2lf\n", hum);
        printf("\tNivel de CO2 no ar: %.2lf\n\n", co2);
        break;
      case 9:
        printf("Opcoes da aplicacao:\n");
        printf("\tDigite 1 para configurar o valor minimo de temperatura da estufa.\n");
        printf("\tDigite 2 para configurar o valor minimo da umidade do solo da estufa.\n");
        printf("\tDigite 3 para configurar o valor minimo do nivel de CO2 da estufa.\n");
        printf("\tDigite 4 para configurar o valor maximo de temperatura da estufa.\n");
        printf("\tDigite 5 para configurar o valor maximo da umidade do solo da estufa.\n");
        printf("\tDigite 6 para configurar o valor maximo do nivel de CO2 da estufa.\n");
        printf("\tDigite 7 para obter os ultimos valores lidos dos sensores.\n");
        printf("\tDigite 0 para encerrar a aplicacao.\n\n");
        break;
      default:
        printf("Opcao invalida. Digite 9 para o menu de ajuda.\n");
        break;
    }
  }
  // Fechando o socket.
  closesocket(clientSocket);
  WSACleanup();
  return 0;
}
