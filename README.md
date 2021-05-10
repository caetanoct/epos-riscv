A primeira coisa feita foi mudar os bits na criação do contexto
usando setando SPP_U, SUM e SPIE em status

Foi alterado o metodo Context::init_stack de forma a salvar o usp de maneira acessivel
no contexto, considerando também um atributo _has_usp que indica se o contexto possui
uma pilha de usuario, o que nem sempre é verdade.

Dessa forma em Context::Load, caso exista pilha de usuario no contexto somamos 
o endereço do proprio contexto com o tamanho de contexto buscando apontar a pilha de usuario
de forma que ela seja carregada no contexto na primeira instrução "move".