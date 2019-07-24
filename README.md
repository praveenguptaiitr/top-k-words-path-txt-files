# top-k-words-path-txt-files
top k words from txt files from path recursively; usage: ./ssfi -t &lt;num of threads> &lt;file path> &lt;num of top K frequents words>

Super Simple File Indexer (ssfi) 
    -> traverse directory recursively and search for txt file 
    -> and search top k words from txt files found in worker threads 
    -> merge all top k words from all the txt file and print only top k words from these files
