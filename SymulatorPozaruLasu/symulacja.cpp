#include <iostream>
#include <vector>
#include <random>
#include <SFML/Graphics.hpp>

// Definicja stanów komórek w siatce
enum State {
    TREE = 0,
    BURNING_TREE = 1,
    BURNED_TREE = 2,
    WATER = 3
};

int main() {
    // Inicjowanie generatora liczb losowych
    srand(static_cast<unsigned int>(time(nullptr)));

    int gridSize;
    float p, ps;
    int k;
    const int cellSize = 10;

    // Pobieranie danych od u¿ytkownika
    // Rozmiar siatki
    do {
        std::cout << "Podaj rozmiar siatki (od 50 do 200): ";
        std::cin >> gridSize;
    } while (gridSize < 50 || gridSize > 200);

    // Prawdopodobieñstwo podpalenia drzewa przez s¹siada
    do {
        std::cout << "Podaj prawdopodobienstwo podpalenia drzewa (0-1): ";
        std::cin >> p;
    } while (p < 0 || p > 1);

    // Prawdopodobieñstwo samozap³onu drzewa
    do {
        std::cout << "Podaj prawdopodobienstwo samozaplonu (0-1): ";
        std::cin >> ps;
    } while (ps < 0 || ps > 1);

    // Liczba iteracji potrzebna do odnowienia siê spalonego drzewa
    std::cout << "Podaj liczbe iteracji do odnowienia sie drzewa: ";
    std::cin >> k;

    // Tworzenie okna symulacji
    sf::RenderWindow window(sf::VideoMode(gridSize * cellSize, gridSize * cellSize), "Fire Simulation");

    // Tworzenie siatki drzew
    sf::Image grid;
    grid.create(gridSize, gridSize, sf::Color::Green);

    // Dodatkowa siatka do œledzenia liczby iteracji od spalenia drzewa
    sf::Image burnTimeGrid;
    burnTimeGrid.create(gridSize, gridSize, sf::Color::Black);

    // Inicjowanie zbiornika wodnego w losowym miejscu
    sf::IntRect waterRect(rand() % (gridSize - 30), rand() % (gridSize - 30), 30, 30);
    for (int x = waterRect.left; x < waterRect.left + waterRect.width; x++) {
        for (int y = waterRect.top; y < waterRect.top + waterRect.height; y++) {
            grid.setPixel(x, y, sf::Color::Blue);
        }
    }

    // Inicjowanie po¿aru w losowym miejscu, ale z dala od wody
    sf::IntRect fireRect;
    bool isWater;
    do {
        fireRect = sf::IntRect(rand() % (gridSize - 4), rand() % (gridSize - 4), 4, 4);
        isWater = false;
        for (int x = fireRect.left; x < fireRect.left + fireRect.width && !isWater; x++) {
            for (int y = fireRect.top; y < fireRect.top + fireRect.height && !isWater; y++) {
                if (grid.getPixel(x, y) == sf::Color::Blue) {
                    isWater = true;
                }
            }
        }
    } while (isWater);
    for (int x = fireRect.left; x < fireRect.left + fireRect.width; x++) {
        for (int y = fireRect.top; y < fireRect.top + fireRect.height; y++) {
            grid.setPixel(x, y, sf::Color::Red);
        }
    }

    // Mo¿liwe kierunki wiatru
    std::vector<sf::Vector2i> wind_directions = {
        {-1, 0}, {1, 0}, {0, -1}, {0, 1}
    };

    int iteration = 0;
    std::vector<sf::Vector2i> current_wind_direction;

    // G³ówna pêtla symulacji
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        // Zmiana kierunku wiatru co 10 iteracji
        if (iteration % 10 == 0) {
            current_wind_direction = { wind_directions[rand() % wind_directions.size()] };
        }

        sf::Image newGrid = grid;
        for (int i = 0; i < gridSize; ++i) {
            for (int j = 0; j < gridSize; ++j) {
                // Jeœli drzewo jest zdrowe, sprawdŸ czy zostanie podpalone przez s¹siada lub samozap³on
                if (grid.getPixel(i, j) == sf::Color::Green) {
                    for (const auto& direction : current_wind_direction) {
                        int di = direction.x;
                        int dj = direction.y;
                        if (i + di >= 0 && i + di < gridSize && j + dj >= 0 && j + dj < gridSize) {
                            if (grid.getPixel(i + di, j + dj) == sf::Color::Red && static_cast<float>(rand()) / RAND_MAX < p) {
                                newGrid.setPixel(i, j, sf::Color::Red);
                            }
                        }
                    }
                    if (static_cast<float>(rand()) / RAND_MAX < ps) {
                        newGrid.setPixel(i, j, sf::Color::Red);
                    }
                }
                // Jeœli drzewo p³onie, zmieñ je na spalone w nastêpnej iteracji
                else if (grid.getPixel(i, j) == sf::Color::Red) {
                    newGrid.setPixel(i, j, sf::Color::Black);
                    burnTimeGrid.setPixel(i, j, sf::Color(iteration, 0, 0)); // Zapisz czas spalenia
                }
                // Jeœli drzewo jest spalone, sprawdŸ czy up³ynê³o wystarczaj¹co du¿o czasu, aby siê odnowiæ
                else if (grid.getPixel(i, j) == sf::Color::Black) {
                    int burnTime = burnTimeGrid.getPixel(i, j).r;
                    if (iteration - burnTime >= k) {
                        newGrid.setPixel(i, j, sf::Color::Green);
                        burnTimeGrid.setPixel(i, j, sf::Color::Black); // Zresetuj czas spalenia
                    }
                }
            }
        }

        grid = newGrid;
        iteration++;

        // SprawdŸ, czy jest jeszcze jakiœ p³on¹cy obszar
        bool isBurning = false;
        for (int i = 0; i < gridSize && !isBurning; ++i) {
            for (int j = 0; j < gridSize && !isBurning; j++) {
                if (grid.getPixel(i, j) == sf::Color::Red) {
                    isBurning = true;
                }
            }
        }
        if (!isBurning) {
            break;
        }

        // Wyœwietl aktualny stan siatki
        window.clear();
        sf::Texture texture;
        texture.loadFromImage(grid);
        sf::Sprite sprite(texture);
        sprite.setScale(cellSize, cellSize);
        window.draw(sprite);
        window.display();

        // Czekaj chwilê przed nastêpn¹ iteracj¹
        sf::sleep(sf::seconds(0.25));
    }

    std::cout << "Symulacja zakonczona." << std::endl;

    return 0;
}






