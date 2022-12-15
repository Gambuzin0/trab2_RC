# trab2_RC

É realizada um verificação do IP recebido na pasv response.
É realizada uma verificação do nº de bytes enviados e recebidos.
É realizada uma verificação da presença de um username e/ou password, no caso de ausencia de qualquer um é atribuido um parâmetro default (user: "anonymous"  | pass: "").
Em alguns casos de erro a resposta dada pela aplicação não é apenas a do servidor!

./out download ftp://ftp.up.pt/pub/kodi/timestamp.txt
./out download ftp://anonymous:@ftp.up.pt/pub/parrot/index.db
./out download ftp://anonymous:@ftp.up.pt/pub/archlinux/iso/2022.12.01/archlinux-x86_64.iso  (2.8G)
./out download ftp://anonymous:@ftp.up.pt/pub/archlinux/iso/2022.12.01/archlinux-bootstrap-x86_64.tar.gz  (173M)
./out download ftp://rcom:rcom@netlab1.fe.up.pt/files/crab.mp4    (É preciso VPN)