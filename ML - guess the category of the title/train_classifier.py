import numpy as np
from sklearn.metrics import confusion_matrix
import matplotlib.pyplot as plt
from sklearn.metrics import multilabel_confusion_matrix
import itertools

def multinomial_naive_bayes_train(X, Y, priors=None):
    X = np.maximum(X, 0).astype(int)  # Force X to be of non-negative integers
    m, n = X.shape
    k = Y.max() + 1
    probs = np.empty((k, n))
    for c in range(k):
        counts = X[Y == c, :].sum(0)
        tot = counts.sum()
        probs[c, :] = (counts + 1) / (tot + n)  # with Laplacian smoothing)
    if priors is None:
        priors = np.bincount(Y) / m
    W = np.log(probs).T
    b = np.log(priors)
    return W, b


def multinomial_naive_bayes_inference(X, W, b):
    scores = X @ W + b.T
    labels = np.argmax(scores, 1)
    return labels




def plot_confusion_matrix(cm,labels,Name): 
    accuracy = np.trace(cm) / float(np.sum(cm))    
    cmap = plt.get_cmap('Greens')
    plt.figure()
    plt.imshow(cm, cmap=cmap)
    plt.title(Name)
    plt.colorbar()    
    number_of_classes = np.arange(len(labels))
    plt.xticks(number_of_classes, labels)
    plt.yticks(number_of_classes, labels)    
    cm = cm.astype('float') / cm.sum(axis=1)[:, np.newaxis]
    for i, j in itertools.product(range(cm.shape[0]), range(cm.shape[1])):
        plt.text(j, i, round(cm[i, j],4),horizontalalignment="center",color="red")
    plt.tight_layout()
    plt.ylabel('Correct label')
    plt.xlabel('Predicted label')

#loading data
data = np.loadtxt('train_made.txt.gz')
X = data[:, :-1].astype(int)
Y = data[:, -1].astype(int)
number=[]
klass=[]
f=open("klasses.txt","r")
for line in f:
    numberV,klassV = line.split()
    klas=klassV.replace("'","")
    klassV=klas.replace(")","")	
    klass.append(klassV)

#Training and checking accuracy + plotting cm
w,b=multinomial_naive_bayes_train(X,Y)
predictions=multinomial_naive_bayes_inference(X,w,b)
accuracy = (predictions == Y).mean()
print("Training accuracy:", accuracy * 100)
cm = confusion_matrix(Y, predictions)
plot_confusion_matrix(cm,klass,"Training Confusion Matrix")

#Loading data and checking accuracy + plotting cm
data = np.loadtxt('test_made.txt.gz')
X = data[:, :-1].astype(int)
Y = data[:, -1].astype(int)
predictions=multinomial_naive_bayes_inference(X,w,b)
accuracy = (predictions == Y).mean()
print("Testing accuracy:", accuracy * 100)
cm = confusion_matrix(Y, predictions)
plot_confusion_matrix(cm,klass,"Testing Confusion Matrix")

#Loading data and checking accuracy + plotting cm
data = np.loadtxt('validation_made.txt.gz')
X = data[:, :-1].astype(int)
Y = data[:, -1].astype(int)
predictions=multinomial_naive_bayes_inference(X,w,b)
accuracy = (predictions == Y).mean()
print("Validation accuracy:", accuracy * 100)
cm = confusion_matrix(Y, predictions)
plot_confusion_matrix(cm,klass,"Validation Confusion Matrix")
plt.show()
