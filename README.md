Quanto ao Loader nossa ideia foi fazer com que o sistema carregasse depois da Task principal outras aplicações em novas 
tasks, fazendo inicialmente uma especie de fork da primeira task e em seguida carregando os segmentos especificos da 
nova task.

Para isso modificamos o Makefile na raiz do projeto para podermos compilar o EPOS passando mais de uma aplicação usando,
por exemplo `make clean APPS='hello task_test philosophers_dinner' run` dessa forma o trabalho de buildar a imagem 
dos apps é feito pelo eposmkbi, e carregá-las pelo thread_init.cc.

Ao tentar carregar os elfs das novas aplicações tivemos problemas, apesar de tentar de varias maneiras não conseguimos 
carregar um elf valido, nesse sentido uma das tentivas foi transformar application_offset em um vetor para pegar o 
offset de cada app em mkbi mas ainda assim o elf gerado não era valido. Conversando com colegas pelo fórum, conseguimos 
ler um elf válido no app_extra, porém criando a nova task a única coisa que acontecia na execução era a duplicação da 
primeira aplicação atribuida. Após isso não conseguimos evoluir muito.

Demos preferencia em se esforçar mais no Loader do que no IPC, a ideia para o segundo seria passar o endereço de um 
Segmento criado por Argv na nova aplicação como mencionado em aula.

Em resumo, o estado atual do nosso EPOS é funcional caso quantidade de apps é 1 e quando apps > 1 o sistema compila 
porém executa o primeiro app duas vezes.
