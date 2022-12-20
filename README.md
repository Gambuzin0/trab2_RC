# Segundo Trabalho Prático RC

Pode-se encontrar vários logs relacionados com algumas das experiências realizadas na pasta "Logs experiências".
Pode-se encontrar alguns documentos relevantes para este trabalho na pasta "docs".
Pode-se encontrar o código-fonte da aplicação de download realizada na pasta "src".

Para testar a aplicação de download, deve-se compilar a o ficheiro main.c usando o comando:

- gcc main.c -o out

E para correr o executável, pode-se utilizar qualquer um destes comandos de exemplo:

./out download ftp://ftp.up.pt/pub/kodi/timestamp.txt
./out download ftp://anonymous:@ftp.up.pt/pub/parrot/index.db
./out download ftp://anonymous:@ftp.up.pt/pub/archlinux/iso/2022.12.01/archlinux-x86_64.iso
./out download ftp://anonymous:@ftp.up.pt/pub/archlinux/iso/2022.12.01/archlinux-bootstrap-x86_64.tar.gz~
./out download ftp://rcom:rcom@netlab1.fe.up.pt/files/crab.mp4    -> É preciso ter a VPN da FEUP ligada

Obviamente, pode ser usado qualquer comando seguindo o formato descrito no relatório/enunciado.