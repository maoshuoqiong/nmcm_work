odex反编译

# odex to smali file
java -jar baksmali-2.2.2.jar x temp.odex

# smali file to dex
java -jar smali-2.2.2.jar out

#dex2jar
