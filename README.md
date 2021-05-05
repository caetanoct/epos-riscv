# README

A entrega começa após implementações de funcionalidades relacionadas
ao p2 como implementação e inicialização de tasks e sua ligação com
as threads.

Após isso, na parte do p3, o framework foi adicionado ao projeto.
Nesse sentido foram implementados os handlers faltantes, os métodos
syscall e syscalled e sua passagem da mensagem por parâmetros através
de IC usando ecall para gerar a exceção, salvando a mensagem no
registrador temporário x31.

Para compilação com o framework foram adicionados os arquivos kernel_binding,
system_binding e system_scaffold, sendo que, nesse sentido ao tentar executar
aplicações ocorreu "undefined reference to `EPOS::S::Machine::pre_init" apontando 
para System_Scaffold o qual não conseguimos descobrir como resolver, dificultando o 
processo de teste e validação das atividades realizadas.

Resolvendo esse problema continuaríamos validando a relação do framework e
o sistema para passagem de syscall e implementaríamos pilhas para cada thread
de kernel de forma a separar o contexto da aplicação (user-land) do sistema
(kernel-space).

Após a apresentação de segunda-feira (03/05), arrumamos os problemas apontados
pelo professor referentes aos parâmetros e retorno da syscall utilizando também
algumas ideias apresentadas pelos colegas, como uso da variavel global estatica para 
a mensagem. Também implementamos pilhas de usuario, baseado no que o professor comentou e tentamos resolver o caso dos métodos de serialização. Contudo não
foi possivel descobrir a causa do problema de referência indefinida a Machine::pre_init, mencionado antes.