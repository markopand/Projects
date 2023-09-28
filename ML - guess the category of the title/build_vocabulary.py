import collections
import numpy as np
import gzip
import nltk

#opening file,extracting data into lists
publisher=[]
all_titles=[]
f = gzip.open('train.txt.gz','rt')
for line in f:
    klassV,publisherV,titleV = line.split('|')
    publisher.append(publisherV)
    all_titles.append(titleV.strip())

voc=collections.Counter()

#adding publishers into titles
for i in range(len(all_titles)):
    all_titles[i]+=' ' + publisher[i]

#using tokenizer to form a vocabulary
tokenizer = nltk.tokenize.WordPunctTokenizer()
for single_title in all_titles:
    voc.update(tokenizer.tokenize(single_title))

#load stopwords
file = 'stopwords.txt'
input_file = open(file,'r')
list_of_stopwords = [word.strip('\n') for word in input_file]


#This list will be used to store used roots
list_of_used_roots = []

#This list will be used to store words:
lemmatizer = nltk.stem.WordNetLemmatizer()

#Lemmatizing each word and removing stopwords from vocabulary
list_of_words = [lemmatizer.lemmatize(word[0].lower()) for word in voc.most_common() if word[0].lower() not in list_of_stopwords]

#avoiding duplicates of the same word
for root in (list_of_words):
    if root not in list_of_used_roots:
        list_of_used_roots.append(root)


#saving vocabulary into a file
f = open("vocabulary.txt","w")
for word in sorted(list_of_used_roots):
    print(word,file=f)
f.close()
