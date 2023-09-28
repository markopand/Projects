import numpy as np
import gzip
import nltk
from nltk.tokenize import sent_tokenize, word_tokenize

#loading vocabulary
def load_vocabulary(filename):
    f = open(filename)
    n = 0
    voc = {}
    for w in f.read().split():
        voc[w] = n
        n += 1
    f.close()
    return voc

#performing BoW on data
def bow(title, voc):
    p = "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~"
    table = str.maketrans(p, " " * len(p))
    title = title.translate(table)
    bow = np.zeros(len(voc))
    for w in title.split():
        if w in voc:
            index = voc[w]
            bow[index] += 1
    return bow

#Lemmatizing the whole sentence
def lemmatizeSentence(title):
    lemmatizer = nltk.stem.WordNetLemmatizer()
    words_after_tokenization=word_tokenize(title)
    lemmatize_title=[]
    for word in words_after_tokenization:
        lemmatize_title.append(lemmatizer.lemmatize(word))
        lemmatize_title.append(" ")
    return "".join(lemmatize_title)


#loading vocabulary and extracting Training data
voc = load_vocabulary("vocabulary.txt")
bow_feature = []
labels = []
klass=np.array([])
publisher=[]
all_titles=[]
unique_klass=np.array([])
f = gzip.open('train.txt.gz','rt')
for line in f:
    klassV,publisherV,titleV = line.split('|')
    klass=np.append(klass,[klassV])
    publisher=np.append(publisher,[publisherV])
    all_titles.append(titleV.strip())
    if(len(unique_klass)==0):
        unique_klass=np.append(unique_klass,[klassV])
    else:
        if(klassV not in unique_klass):
            unique_klass=np.append(unique_klass,[klassV])
f.close()

#Changing names of classess with numbers
for j in range(len(unique_klass)):
    klass=np.where(klass==unique_klass[j],j,klass)

#saving classess and assigned numbers to a file
f = open("klasses.txt","w")
for k in range(len(unique_klass)):
    print((k,unique_klass[k]),file=f)
f.close()

#adding publishers to titles
for i in range(len(all_titles)):
    all_titles[i]+=' '+publisher[i]

#performing BoW and stemming on all of the training data
for title in all_titles:
    bow_feature.append(bow(lemmatizeSentence(title.lower()),voc))


# np.stack transforms the list of vectors into a 2D array.
X = np.array(bow_feature).astype(int)
Y = np.array(klass).astype(int)

# The following line append the labels Y as additional column of the
# array of features so that it can be passed to np.savetxt.
data = np.concatenate([X, Y[:, None]], 1)
np.savetxt('train_made.txt.gz', data)


#TEST data

#Extracting Test data
bow_feature = []
labels = []
klass=np.array([])
publisher=[]
all_titles=[]
f = gzip.open('test.txt.gz','rt')
for line in f:
    klassV,publisherV,titleV = line.split('|')
    klass=np.append(klass,[klassV])
    publisher=np.append(publisher,[publisherV])
    all_titles.append(titleV.strip())

f.close()

#Changing names of classess with numbers
for j in range(len(unique_klass)):
    klass=np.where(klass==unique_klass[j],j,klass)

#Adding publishers to titles
for i in range(len(all_titles)):
    all_titles[i]+=' '+publisher[i]

#performing BoW and stemmization of the Test data
for title in all_titles:
    bow_feature.append(bow(lemmatizeSentence(title.lower()),voc))



# np.stack transforms the list of vectors into a 2D array.
X = np.array(bow_feature).astype(int)
Y = np.array(klass).astype(int)

# The following line append the labels Y as additional column of the
# array of features so that it can be passed to np.savetxt.
data = np.concatenate([X, Y[:, None]], 1)
np.savetxt('test_made.txt.gz', data)

#Validation data

##Extracting Validation data
bow_feature = []
labels = []
klass=np.array([])
publisher=[]
all_titles=[]
f = gzip.open('validation.txt.gz','rt')
for line in f:
    klassV,publisherV,titleV = line.split('|')
    klass=np.append(klass,[klassV])
    publisher=np.append(publisher,[publisherV])
    all_titles.append(titleV.strip())
f.close()

#Changing names of classess with numbers
for j in range(len(unique_klass)):
    klass=np.where(klass==unique_klass[j],j,klass)

#Adding publishers to titles
for i in range(len(all_titles)):
    all_titles[i]+=' '+publisher[i]

#performing BoW and stemmization of the Validation data
for title in all_titles:
    bow_feature.append(bow(lemmatizeSentence(title.lower()),voc))



# np.stack transforms the list of vectors into a 2D array.
X = np.array(bow_feature).astype(int)
Y = np.array(klass).astype(int)

# The following line append the labels Y as additional column of the
# array of features so that it can be passed to np.savetxt.
data = np.concatenate([X, Y[:, None]], 1)
np.savetxt('validation_made.txt.gz', data)
