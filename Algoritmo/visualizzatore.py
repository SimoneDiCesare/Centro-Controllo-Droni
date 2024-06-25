import numpy as np
import matplotlib.pyplot as plt

def visualizza(matrice):
    plt.clf()  # Cancella la figura precedente
    plt.imshow(matrice, cmap='jet')  # Puoi cambiare il colormap a tuo piacimento
    plt.colorbar()  # Aggiungi una barra dei colori per la scala
    plt.draw()
    plt.pause(0.03)  # Aggiorna il grafico per 0.1 secondi per mostrare le modifiche

def salva(matrice):
    plt.imsave('matricePene.png', matrice, cmap='jet')


# Esempio di utilizzo
if __name__ == "__main__":
    ''' # Definisci la tua matrice
    matrice_input = np.random.rand(10, 100, 200)  # Esempio di matrice casuale 10x10x10
                                #n frame, altezza, larghezza
    # Normalizza la matrice per far sì che i valori siano compresi tra 0 e 1
    matrice_normalizzata = matrice_input / np.max(matrice_input)

    # Avvia l'animazione
    animate(matrice_normalizzata)
    '''

    matrice_input = np.random.rand(10, 10)  # Esempio di matrice casuale 10x10

    # Visualizza l'immagine colorata della matrice all'avvio
    visualizza(matrice_input)

  

