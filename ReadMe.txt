---------------------------------------------------------------------
Dascalu Stefan-Teodor
324CC
---------------------------------------------------------------------
                          E. L. F. Loader
---------------------------------------------------------------------

    Pornind de la scheletul primit, folosim structurile so_exec_t
si cele specifice functiei sigaction. Initializam un contor cu 0,
acesta ne va arata daca campul data al fiecarui segment a fost alocat.
Odata alocate contorul va fi setat cu 1 si for-ul nu va mai repeta
alocarea.
    Iteram prin segmentele fisierlui si cautam daca adresa la care
s-a produs pageFault-ul se afla in segmentul curent 'i'. Salvam
marimea unei pagini si calculam a cata pagina din segment s-a produs
pageFault-ul.
    Cu ajutorul lui int buf, verificam daca pagina pageNr a fost mapata,
buf copiaza memoria din segment[i].data (un pseudo vector de frecventa,
care stocheaza nr de pagini inturi, acestea pot fi 1 sau 0).
    Se mapeaza intreaga pagina si se trateaza cele doua cazuri, cel in care
file_zise se afla in interiorul paginii pageNr (daca da, se umple cu 0 memoria
de la file_zise pana la finalul paginii) si cazul in care intraga pagina
se afla dupa file_zise astfel, setand 0 pe toata memoria segmentului.
    Dupa mapare si eventuala zeroire, protejam zona de memorie si setam
zona din data corespunzatoare cu 1.
    In cazul in care pagina a fost deja mapata, sau daca nu se gaseste
intr-un segment al fisierului se utilizeaza handlerul default.