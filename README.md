# P2

## Mapeamento de memória

Primeiro mapeamos os endereços da memória com base no modelo do SiFive_U no arquivo sifive_e_traits.h.

## Setup

Depois implementamos o setup do SiFive_E, também baseado no setup do SiFive_U, porém essa implementação não possuia os métodos setup_sys_pt e setup_sys_pd que estavam declarados, então copiamos do setup do LegacyPC e removemos as partes que referenciavam partes exclusivas da arquitetura e que não estavam presentes.

## Executando

Após isso, quando executado, o EPOS compilava normalmente e gerava os arquivos de imagem para setup, system e init, porém ao começar a execução o programa ficava parado sem executar até acabar o tempo de execução da aplicação.

### O que deu errado?

Tentando usar o comando de debug do EPOS, era retornado um erro não encontrando o comando konsole, declarado no makedefs.
Usando o trace, foi percebido que a execução parava no método load_parts() do setup, e com alguns comandos de impressão de debugs adicionados manualmente, o método responsável pela parada era o memcpy. Provavelmente causado por algum erro na criação das páginas.
