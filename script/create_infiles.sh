#! /bin/bash
START=$(date +%s) # starting time
inputFile=$1
input_dir=$2
numFilesPerDirectory=$3
declare -a CountriesArr
declare -A numFiles;

if [ $# -ne 3 ];
then
    printf "3 Arguments must be given!\n"
    exit
elif [ ! -s $inputFile ];
then
    printf "inputFile error!\n"
    exit
elif ! [[ $numFilesPerDirectory =~ ^[0-9]+$ ]];
then
    printf "numFilesPerDirectory error!\n"
    exit
elif [ $numFilesPerDirectory -lt 0 ];
then
    printf "numFilesPerDirectory error!\n"
    exit
elif [ $numFilesPerDirectory -eq 0 ];
then
    printf "numFilesPerDirectory error!\n"
    exit

fi

mkdir -p $input_dir
cd $input_dir
 
while IFS= read -r line;
do
    subDirectory=$(printf %s "$line" | awk '{print $4}')
    if [ ! -d $subDirectory ];
    then
        CountriesArr+=("$subDirectory")
        mkdir $subDirectory 
        cd $subDirectory
        for(( currFile=0; currFile < $numFilesPerDirectory; currFile++ ));
        do
            printf "" > ${subDirectory}-$((${currFile}+1)).txt
            numFiles+=([$subDirectory]=1)
        done
        cd ../  
    fi
done < ../$inputFile

printf "Created directories and files\n"

while IFS= read -r line;
do 
    IFS=' ' read -r -a splitedLine <<< $line
    country=${splitedLine[3]}
    cd $country
    printf %s "$line" >> ${country}-${numFiles[$country]}.txt
    echo "" >> ${country}-${numFiles[$country]}.txt
    if [ $((${numFiles[$country]} % $numFilesPerDirectory)) -eq 0 ];
    then       
        numFiles+=([$country]=0)
    fi
    numFiles+=([$country]=$((${numFiles[$country]}+1)))
    cd ../
done < ../$inputFile

printf "\n"
END=$(date +%s)
DIFF=$(( $END - $START ))
printf "Done: It took $DIFF seconds\n"