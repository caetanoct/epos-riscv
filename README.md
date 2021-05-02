A entrega começa após implementações de funcionalidades relacionadas 
ao p2 como implementação e inicialização de tasks e sua ligação com 
as threads.

Após isso, na parte do p3, o framework foi adicionado ao projeto. 
Nesse sentido foram implementados os handlers faltantes, os métodos 
syscall e syscalled e sua passagem da mensagem por parâmetros através 
de IC usando ecall para gerar a exceção, salvando a mensagem no 
registrador temporário x31.

Na compilação foram resolvidas questões como os arquivos kernel_binding, 
system_binding e system_scaffold, ausentes no nosso projeto, sendo no 
que não conseguimos descobrir como resolver.

Resolvendo esse problema continuaríamos validando a relação do framework e 
o sistema para passagem de syscall e implementaríamos pilhas para cada thread 
de kernel de forma a separar o contexto da aplicação (user-land) do sistema 
(kernel-space). 

