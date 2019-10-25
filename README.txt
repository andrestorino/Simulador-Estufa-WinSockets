O código foi feito em lingugagem C no sistema operacional Windows 10 (embora ele deva funcionar em outras versões do Windows).

Utilize uma IDE para compilar os arquivos .c e executar os programas separadamente.
Comandos de compilação (caso deseje compilar na mão):

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

É necessário permitir

Foi usado o IP de localhost da maquina para fazer a simulacao.

Nao foi usado json como especificado no protocolo.

O gerenciador buga caso a conexao com algum componente seja fechada abruptamente (ie o programa rodando algum componente for fechado), pois o socket ainda estaria aberto. Caso o programa de algum componente for finalizado de maneira correta (o cliente por exemplo, por meio da opção 0) e o socket for fechado, o gerenciador continua rodando, podendo inclusive aceitar que outra conexao com o cliente seja estabelecida no futuro. Porém, só é possível fechar o cliente dessa forma pois é esperado que os sensores não parem de funcionar e o socket nunca se feche.

Foram gerados e somados números aleatórios na leitura dos sensores para que se pudesse ter alterações que causassem algum tipo de resposta do gerenciador. Os atuadores recebem a mensagem de liga/desliga do gerenciador a medida que é necessário, e imprimem em seus consoles quando são ativados ou desativados.

Os atuadores em si não fazem nada para modificar a leitura dos sensores, tendo em vista que isso iria precisar que novas mensagens não definidas no protocolo fossem criadas só para esta simulação. O objetivo era que eles recebessem o sinal de liga desliga e, em algum caso real, que eles conseguissem modificar a leitura dos sensores conforme necessário, e esse objetivo foi alcançado.

Foi disponibilizado um print dos processos rodando (teste.png)
