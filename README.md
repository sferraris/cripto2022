# cripto2022

## Compilación
Para poder compilar este programa se debe tener gcc instalado, y la librería de
openssl descargada en la computadora. 
#### Es importante que la versión de openssl que tengan descargada tenga la implementación de des en env.h. Hay algunas versiones que la tienen en provider.h, si tu versión tiene esto, el programa no podrá utilizar el método des.
Se compila con el siguiente comando:
#### make all

Se elimina el ejecutable con el siguiente comando:
#### make clean


## Parámetros obligatorios:

➢ -embed 

Indica que se va a ocultar información.

➢ -in file

Archivo que se va a ocultar.

➢ -p bitmapfile

Archivo bmp que será el portador.

➢ -out bitmapfile

Archivo bmp de salida, es decir, el archivo bitmapfile con la información de file
incrustada.

➢ -steg <LSB1 | LSB4 | LSBI>

algoritmo de esteganografiado: LSB de 1bit, LSB de 4 bits, LSB Enhanced

## Parámetros opcionales

➢ -a <aes128 | aes192 | aes256 | des>

➢ -m <ecb | cfb | ofb | cbc>

➢ -pass password (password de encripcion)

#### IMPORTANTE: TODOS LOS PARÁMETROS DEBEN ESTAR EN EL ORDEN ESCRITO

#### Ejemplo de ejecución
./stegobmp –extract –p “imagenmas1 .bmp” -out “mensaje1” –steg LSBI –a des –m cbc -pass
“oculto”


