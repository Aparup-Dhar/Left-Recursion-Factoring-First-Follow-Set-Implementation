# Title
Left Recursion Factoring First Follow Set Implementation

# Description
Left Recursion Factoring First Follow Set Implementation using vanilla C language.

# Features

`▪ Left Recursion`
  
`▪ Left Factoring`

`▪ First Sets`

`▪ Follow Sets`

# Technologies Used
![C](https://img.shields.io/badge/C-A8B9CC?style=for-the-badge&logo=c&logoColor=white)

# Example Input
```shell
Enter Number of Productions: 3
Enter the grammar:
A->aB|Ad
B->b
C->g
```

# Example Output
```shell
GRAMMAR: A->aB|Ad

Grammar after removing left recursion:
A->aBX
X->dX|e


GRAMMAR: B->b

No left recursion found.
B->b


GRAMMAR: C->g

No left recursion found.
C->g


GRAMMAR: A->aBX


No common prefix found.
A->aBX


GRAMMAR: X->dX|e


No common prefix found.
X->dX|e


GRAMMAR: B->b


No common prefix found.
B->b


GRAMMAR: C->g


No common prefix found.
C->g
First sets:
First(A) = { a }
First(X) = { d e }
First(B) = { b }
First(C) = { g }

Follow sets:
Follow(A) = { $ }
Follow(B) = { d e}
Follow(X) = { $ }
Follow(C) = { }
```
