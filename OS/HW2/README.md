<h1 align="center">Threads and Syncronization</h1>
<p align="center"><strong>Introduction to Operating Systems - Homework 2</strong>
</p>
<h2>About</h2>
This homework is focused on threads and their syncronization over a matrix of integers. The scenario, in general, is there are "proper privates" where each of them associated with a thread, tries to clean certain areas of the garden. The garden is basically a integer matrix that is given as an input to the program. While a private is cleaning, it should disable all other privates that have areas intersecting with its' area. Also there is a commander which gives orders like "stop", "continue" and "break" and each order should be obeyed immediately. Lastly, there are smokers in the garden which are increasing the values in the garden matrix one by one. Details of how the cleaning and smoking is done can be found in the homework pdf. The aim is to protect the integrity of the garden while avoiding any deadlock between threads.

<h2>Requirements</h2>

- C/C++
- Mandatory usage of only pthread library regarding syncronization
- Use of predefined notifiers for testing
- Input ends with EOF

<h2>Key learnings</h2>

- Mutexes
- Condition variables
- pthread creation, join

