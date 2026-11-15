# Submission

## Members
### Member 1
- Name:MINAS KONSTANTINOS 
- Academic ID:ics21047

### Member 2
- Name: 
- Academic ID:

## Σύντομη περιγραφή υλοποίησης
Η υλοποιηση υποστηριζει ολα τα βηματα του assignment 1. Επιπλεον εκτος 
απο την αποθηκευση του path που βρισκεται το shell ενημερωνει το $PWD. 
Υποστηριζεται επισης pipeline με ανακατευθυνσεις. Εξαιρεση pipeline με build in 
commands οπως cd.

Ανακατευθυνσεις που υποστηριζονται(>, < ,>>)

Για την υλοποιση του pipeline δημιουργηται πινακας με pipes
αναλογα με το πληθος των εντολων. Υπαρχει ελεγχος σε περιπτωση που αποτυχει καποιο 
pipe ωστε να κλεισουμε τα υπολοιπα.

Background jobs υποστηριζονται με την χρηση του ετοιμου πινακα jobs. Προβλημα με 
race condition οταν το child process αποτυγχανει και το parent process δεν προλαβαινει 
να ενημερωσει το πινακα με τα jobs.

Στην περιπτωση που το pipeline λειτουργει στο background δεν χρησιμοποιειται 
wait στην execute pipeline, μονο στο reap_bg_jobs.





Tests:
pwd
ls
ls -l
cd /tmp
pwd
echo "Hello from Minish"
echo "Shopping list" > minish-demo.txt
echo "milk" >> minish-demo.txt
echo "bread" >> minish-demo.txt
cat minish-demo.txt
cat < minish-demo.txt
ls /usr/bin | head -n 5
cat minish-demo.txt | grep milk
cat minish-demo.txt | tr a-z A-Z
ls /usr/bin | sort | head -n 5
ls /usr/bin | head -n 5 > minish-programs.txt
cat minish-programs.txt
sleep 10 &
echo "I can still run commands"
cat /this-file-does-not-exist
echo "The shell still works"
cd
pwd
exit
