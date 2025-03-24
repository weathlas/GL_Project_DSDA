L'intersection de 2 plan forme une ligne


- CAS 1:
    La ligne n'est pas une ligne à l'infini, les 2 plans ne sont pas paralleles

    Dans ce cas, il y a 2 issues:
    1. Soit la ligne ne traverse aucun triangle: il n'y a pas de collision
    2. Soit la ligne passe à travers les triangles: cas de d'intersection des triangles


    La ligne d'intersection étant coplanaire aux triangles, on peut projeter les sommets sur la ligne

    Projeter les points des triangles sur la ligne (ou sur un plan perpendiculaire au triangle qui passe par la droite)

    On peut donc ensuite observer les sens de translation causé par la projection
    
    Cas 1:
    - Si les 3 translation d'un triangle sont dans la meme direction, cela signifie que tout le triangle est d'un coté de la droite -> pas d'intersction

    Cas 2:
    - Si les directions sont différents pour les 2 triangles, cela signifie que la droite intersecte les triangle
    - On est aussi dans ce cas si l'une de translation à une amplitude de 0: la droite passe par l'un des points du triangle (collision parfaite)

    En general une collision de 2 convex hull corespond à un autre convex hull qui coreposnd à l'espace d'intersection booléen

    Dans le cas de la collison il peut être intéréssant de former cette forme, qui va servir par la suite à la résolution de la collison
    Dans ce cas 2, il faut se souvenir de ce segment (intersection des 2 triangles)


    Après la détection, il nous reste 2




- CAS 2:
    La ligne est une ligne à l'infini:

    - Cas 2.1:
        Les deux 



 (peut etre une ligne à l'infini. dans ce cas, il n'y a pas de collision -> les plan sont paralleles