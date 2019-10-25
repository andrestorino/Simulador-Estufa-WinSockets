#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <windows.h>
#include <ws2tcpip.h>
#pragma comment (lib, "ws2_32.lib")
#define TRUE 1
#define FALSE 0
// Configuracoes do servidor.
#define PORT 7640  // Porta aleatoria
#define MSG_LENGTH 64
#define MAX_CLIENTS 8
// IDs previamente definidos dos componentes.
#define ID_SEN_TEMP 0
#define ID_SEN_UMI 1
#define ID_SEN_CO2 2
#define ID_ATU_AQ 3
#define ID_ATU_RESF 4
#define ID_ATU_IRR 5
#define ID_ATU_INJ 6
#define ID_CLIENT 7
// Configuracoes iniciais da estufa.
#define DEFAULT_MIN_TEMP 15.0
#define DEFAULT_MIN_UMI 0.15
#define DEFAULT_MIN_CO2 300.0
#define DEFAULT_MAX_TEMP 35.0
#define DEFAULT_MAX_UMI 0.35
#define DEFAULT_MAX_CO2 600.0
#define MAX_INVALID_READINGS 10
// Possiveis tipos de requisicoes do cliente
#define REQ_MIN_TEMP 1
#define REQ_MIN_UMI 2
#define REQ_MIN_CO2 3
#define REQ_MAX_TEMP 4
#define REQ_MAX_UMI 5
#define REQ_MAX_CO2 6
#define REQ_READING 7

int main(int argc, char const *argv[]) {
  // Variaveis utilizadas na criacao e configuracao do servidor.
  int serverSocket;  // Socket do servidor.
  int newSocket;  // Socket que ira se conectar ao servidor.
  int valRead;  // Valor de retorno durante a leitura de uma nova mensagem.
  int clientSockets[MAX_CLIENTS];  // Vetor com o identificador de cada cliente.
  char opt = 1;  // Habilita opcoes booleanas na funcao setsockopt.
  char buffer[MSG_LENGTH] = {0};  // Buffer para recebimento e envio de dados.
  struct sockaddr_in addr;  // Estrutura para as configuracoes de conexao.
  int addrLen;  // Variavel para armazenar o tamanho da estrutura sockaddr_in
  WSADATA _data;  // Usada no carregamento da DLL do winsock.
  fd_set readFDS;  // Conjunto com os descritores de arquivo dos sockets conectados.
  int maxSD;  // Descritor de socket com valor maximo.
  int sd;  // Descritor de socket;
  int activity;  // Usado no select. Indica se algo ocorreu em algum socket.
  int id;  // ID para identificar o novo componente conectado.
  int i;
  // Variaveis utilizadas para a gerencia da estufa.
  double minTemperature = DEFAULT_MIN_TEMP;
  double minHumidity = DEFAULT_MIN_UMI;
  double minCO2 = DEFAULT_MIN_CO2;
  double maxTemperature = DEFAULT_MAX_TEMP;
  double maxHumidity = DEFAULT_MAX_UMI;
  double maxCO2 = DEFAULT_MAX_CO2;
  double lastTemperature;
  double lastHumidity;
  double lastCO2;
  char isHeaterOn = FALSE;
  char isCoolerOn = FALSE;
  char isIrrigatorOn = FALSE;
  char isInjectorOn = FALSE;
  // O atuador sera ligado se ocorrerem MAX_INVALID_READINGS leituras seguidas fora dos limites.
  double temperatureCounter = 0;
  double humidityCounter = 0;
  double CO2Counter = 0;
  // Obtendo dados da requisicao do cliente
  int req;
  double updateValue;

  // Inicializando vetor de identificacao dos clientes com 0.
  memset(clientSockets, 0, MAX_CLIENTS * sizeof(int));
  // Carregando a DLL do winsock.
  if (WSAStartup(MAKEWORD(2,0), &_data) == SOCKET_ERROR) {
    fprintf(stderr, "Erro: Winsock DLL nao foi encontrada.\n");
    exit(EXIT_FAILURE);
  }
  // Criando o descritor do socket.
  if ((serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == 0){
    fprintf(stderr, "Erro na criacao do socket.\n");
    exit(EXIT_FAILURE);
  }
  printf("Aguardando conexao...\n");
  // Configurando o socket.
  if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
    fprintf(stderr, "Erro ao configurar o socket.\n");
    exit(EXIT_FAILURE);
  }
  // Definindo a porta do socket como a porta escolhida.
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(PORT);
  // Dando bind no socket.
  if (bind(serverSocket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    fprintf(stderr, "Erro ao dar bind no socket.\n");
    exit(EXIT_FAILURE);
  }
  // Esperando ate um numero maximo de conexoes.
  if (listen(serverSocket, MAX_CLIENTS) < 0) {
    fprintf(stderr, "Erro ao esperar por conexoes novas.\n");
    exit(EXIT_FAILURE);
  }

  // Iniciando loop para aceitar conexoes dos sensores, atuadores e do cliente.
  addrLen = sizeof(struct sockaddr_in);
  while (TRUE) {
    // Limpando o conjunto de sockets
    FD_ZERO(&readFDS);
    // Adicionando o socket do servidor ao conjunto.
    FD_SET(serverSocket, &readFDS);
    maxSD = serverSocket;
    // Adicionando sockets dos clientes ao conjunto.
    for (i = 0; i < MAX_CLIENTS; i++) {
      // Descritor do socket.
      sd = clientSockets[i];
      // Se o descritor de socket for valido, entao adicione-o no conjunto.
      if (sd > 0) FD_SET(sd, &readFDS);
      // Atualizando descritor de socket com maior valor. Utilizado na funcao select.
      if(sd > maxSD) maxSD = sd;
    }
    // Esperando indefinidamente por alguma atividade em um dos sockets
    activity = select(maxSD + 1, &readFDS, NULL, NULL, NULL);
    if ((activity < 0) && (errno != EINTR)) {
      fprintf(stderr, "Erro no select");
    }
    // Se um evento ocorreu no socket do servidor, entao houve uma tentativa de conexao.
    if (FD_ISSET(serverSocket, &readFDS)) {
      if ((newSocket = accept(serverSocket, (struct sockaddr *)&addr, &addrLen)) < 0) {
        fprintf(stderr, "Erro ao aceitar uma conexao nova.\n");
        exit(EXIT_FAILURE);
      }
      printf("Nova conexao:\n\tSocket: %d\n\tIP: %s\n\tPort: %d\n", newSocket, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
      // Realizando identificacao da nova conexao.
      memset(buffer, 0, MSG_LENGTH * sizeof(char));
      valRead = recv(newSocket, buffer, MSG_LENGTH, 0);
      // Lendo o ID recebido.
      sscanf(buffer, "%d", &id);
      // Enviando confirmacao se o ID for reconhecido.
      if (id < 0 || id > 7) send(newSocket, "1", strlen("1"), 0);
      else {
        send(newSocket, "0", strlen("0"), 0);
        // Adicionando o novo socket ao vetor de sockets de clientes
        clientSockets[id] = newSocket;
      }
    }
    // Caso contrario, ocorreu alguma operacao de entrada e saida com outro socket.
    for (i = 0; i < MAX_CLIENTS; i++) {
      sd = clientSockets[i];
      if (FD_ISSET(sd, &readFDS)) {
        // Checar se o socket fechou a conexao e ler a mensagem.
        memset(buffer, 0, MSG_LENGTH * sizeof(char));
        if ((valRead = recv(sd, buffer, MSG_LENGTH, 0)) == 0) {
          // Alguem se desconectou.
          getpeername(sd, (struct sockaddr *)&addr, &addrLen);
          printf("Um host de desconectou:\n\tIP: %s\n\tPort: %d\n",
                 inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
          // Fechando o socket e marcando 0 no vetor de descritores de socket.
          closesocket(sd);
          clientSockets[i] = 0;
        }
        // Identificar o remetente da mensagem e agir de acordo
        else {
          switch (i) {
            case ID_SEN_TEMP:  // Sensor de temperatura.
              printf("Mensagem recebida do sensor de temperatura: %s\n", buffer);
              sscanf(buffer, "%lf", &lastTemperature);
              if (lastTemperature > maxTemperature && isCoolerOn == FALSE) {
                if (++temperatureCounter >= MAX_INVALID_READINGS) {
                  printf("Temperatura ficou acima do valor maximo por %d segundos consecutivos.\n",
                         MAX_INVALID_READINGS);
                  send(clientSockets[ID_ATU_RESF], "1", strlen("1"), 0);
                  isCoolerOn = TRUE;
                  printf("O resfriador foi ligado.\n");
                  temperatureCounter = 0;
                }
              }
              else if (lastTemperature < minTemperature && isHeaterOn == FALSE) {
                if (++temperatureCounter >= MAX_INVALID_READINGS) {
                  printf("Temperatura ficou abaixo do valor minimo por %d segundos consecutivos.\n",
                         MAX_INVALID_READINGS);
                  send(clientSockets[ID_ATU_AQ], "1", strlen("1"), 0);
                  isHeaterOn = TRUE;
                  printf("O aquecedor foi ligado.\n");
                  temperatureCounter = 0;
                }
              }
              if (lastTemperature < ((maxTemperature + minTemperature) / 2) && isCoolerOn) {
                send(clientSockets[ID_ATU_RESF], "0", strlen("0"), 0);
                isCoolerOn = FALSE;
                printf("O resfriador foi desligado.\n");
              }
              if (lastTemperature > ((maxTemperature + minTemperature) / 2) && isHeaterOn) {
                send(clientSockets[ID_ATU_AQ], "0", strlen("0"), 0);
                isHeaterOn = FALSE;
                printf("O aquecedor foi desligado.\n");
              }
              break;
            case ID_SEN_UMI:  // Sensor de umidade.
              printf("Mensagem recebida do sensor de umidade: %s\n", buffer);
              sscanf(buffer, "%lf", &lastHumidity);
              if (lastHumidity < minHumidity && isIrrigatorOn == FALSE) {
                if (++humidityCounter >= MAX_INVALID_READINGS) {
                  printf("Umidade do solo ficou abaixo do valor minimo por %d segundos consecutivos.\n",
                         MAX_INVALID_READINGS);
                  send(clientSockets[ID_ATU_IRR], "1", strlen("1"), 0);
                  isIrrigatorOn = TRUE;
                  printf("O sistema de irrigacao foi ligado.\n");
                  humidityCounter = 0;
                }
              }
              if (lastHumidity > ((maxHumidity + minHumidity) / 2) && isIrrigatorOn) {
                send(clientSockets[ID_ATU_IRR], "0", strlen("0"), 0);
                isIrrigatorOn = FALSE;
                printf("O sistema de irrigacao foi desligado.\n");
              }
              break;
            case ID_SEN_CO2:  // Sensor de CO2.
              printf("Mensagem recebida do sensor de CO2: %s\n", buffer);
              sscanf(buffer, "%lf", &lastCO2);
              if (lastCO2 < minCO2 && isInjectorOn == FALSE) {
                if (++CO2Counter >= MAX_INVALID_READINGS) {
                  printf("Nivel de CO2 no ar ficou abaixo do valor minimo por %d segundos consecutivos.\n",
                         MAX_INVALID_READINGS);
                  send(clientSockets[ID_ATU_INJ], "1", strlen("1"), 0);
                  isInjectorOn = TRUE;
                  printf("O injetor de CO2 foi ligado.\n");
                  CO2Counter = 0;
                }
              }
              if (lastCO2 > ((maxCO2 + minCO2) / 2) && isInjectorOn) {
                send(clientSockets[ID_ATU_INJ], "0", strlen("0"), 0);
                isInjectorOn = FALSE;
                printf("O injetor de CO2 foi desligado.\n");
              }
              break;
            case ID_CLIENT:  // Cliente.
              printf("Mensagem recebida do cliente: %s\n", buffer);
              sscanf(buffer, "%d", &req);
              switch (req) {
                case REQ_MIN_TEMP:
                  sscanf(buffer, "%*d %lf", &minTemperature);
                  break;
                case REQ_MIN_UMI:
                  sscanf(buffer, "%*d %lf", &minHumidity);
                  break;
                case REQ_MIN_CO2:
                  sscanf(buffer, "%*d %lf", &minCO2);
                  break;
                case REQ_MAX_TEMP:
                  sscanf(buffer, "%*d %lf", &maxTemperature);
                  break;
                case REQ_MAX_UMI:
                  sscanf(buffer, "%*d %lf", &maxHumidity);
                  break;
                case REQ_MAX_CO2:
                  sscanf(buffer, "%*d %lf", &maxCO2);
                  break;
                case REQ_READING:
                  memset(buffer, 0, MSG_LENGTH * sizeof(char));
                  snprintf(buffer, MSG_LENGTH, "%.2lf %.2lf %.2lf", lastTemperature, lastHumidity, lastCO2);
                  send(sd, buffer, strlen(buffer), 0);
                  break;
              }
              printf("Requisicao atendida.\n");
              break;
            default:
              break;
          }
        }
      }
    }
  }
  closesocket(serverSocket);
  WSACleanup();
  return 0;
}
