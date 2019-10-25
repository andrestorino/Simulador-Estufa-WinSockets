Integrantes do grupo:

Gabriel Francischini de Souza 9361052
Andr� Luis Storino Junior 9293668
Kau� Lopes de Moraes 9277576

README:
O c�digo foi feito em lingugagem C no sistema operacional Windows 10 (embora ele deva funcionar em outras vers�es do Windows).

Utilize uma IDE para compilar os arquivos .c e executar os programas separadamente.
Comandos de compila��o (caso deseje compilar na m�o):

gcc gerenciador.c -o gerenciador -lwsock32
gcc cliente.c -o cliente -lwsock32
gcc atuadorAquecedor.c -o atuadorAquecedor -lwsock32
gcc atuadorResfriador.c -o atuadorResfriador -lwsock32
gcc atuadorIrriga.c -o atuadorIrriga -lwsock32
gcc atuadorInjCO2.c -o atuadorInjCO2 -lwsock32
gcc sensorTemp.c -o sensorTemp -lwsock32
gcc sensorUmi.c -o sensorUmi -lwsock32
gcc sensorCO2.c -o sensorCO2 -lwsock32

Usar flag -lwsock32 para compilar

Ligar primeiro o gerenciador, depois preferencialmente o cliente (para configurar caso desejado), os atuadores, e depois os sensores.

� necess�rio permitir

Foi usado o IP de localhost da maquina para fazer a simulacao.

Nao foi usado json como especificado no protocolo.

O gerenciador buga caso a conexao com algum componente seja fechada abruptamente (ie o programa rodando algum componente for fechado), pois o socket ainda estaria aberto. Caso o programa de algum componente for finalizado de maneira correta (o cliente por exemplo, por meio da op��o 0) e o socket for fechado, o gerenciador continua rodando, podendo inclusive aceitar que outra conexao com o cliente seja estabelecida no futuro. Por�m, s� � poss�vel fechar o cliente dessa forma pois � esperado que os sensores n�o parem de funcionar e o socket nunca se feche.

Foram gerados e somados n�meros aleat�rios na leitura dos sensores para que se pudesse ter altera��es que causassem algum tipo de resposta do gerenciador. Os atuadores recebem a mensagem de liga/desliga do gerenciador a medida que � necess�rio, e imprimem em seus consoles quando s�o ativados ou desativados.

Os atuadores em si n�o fazem nada para modificar a leitura dos sensores, tendo em vista que isso iria precisar que novas mensagens n�o definidas no protocolo fossem criadas s� para esta simula��o. O objetivo era que eles recebessem o sinal de liga desliga e, em algum caso real, que eles conseguissem modificar a leitura dos sensores conforme necess�rio, e esse objetivo foi alcan�ado.

Foi disponibilizado um print dos processos rodando (teste.png)
