#!/bin/bash
command -v make >/dev/null 2>&1 || { echo >&2 "Necesito make, pero no esta!!"; exit 1; }
command -v sudo >/dev/null 2>&1 || { echo >&2 "Necesito sudo, pero no esta!!"; exit 1; }

echo "Instalando so-commons"
cd /tmp
rm -rf so-commons-library
git clone https://github.com/sisoputnfrba/so-commons-library.git
cd so-commons-library
sudo make install


echo "Instalando ansisop-parser"
cd /tmp
rm -rf ansisop-parser
git clone https://github.com/sisoputnfrba/ansisop-parser.git
cd ansisop-parser/parser/
make all
sudo make install


echo "Fin.."
