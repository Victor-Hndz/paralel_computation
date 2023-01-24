#!/bin/bash
echo "Introduce el numero de ejecuciones a procesar: "
read ejecuciones

#Generamos los datos
./generador "datos.raw" $((ejecuciones*100)) $((ejecuciones*100))

#Borramos el contenido del fichero
> "resultado.csv"

#Escribimos en el fichero los datos de las columnas
echo "Filas,Columnas,Tiempo_ensamblador,Tiempo_en_c,Diferencia,Porcentaje_de_mejora,Mas_rapido">> "resultado.csv"

aumento=100 #Variable que aumenta de 100 en 100
for ((i=0; i<ejecuciones; i++))
do
    ./main "datos.raw" $aumento $aumento "no" "si">> "resultado.csv"; #Ejecutamos el programa y escribimos los datos en el fichero
    aumento=$((aumento+100))
done

