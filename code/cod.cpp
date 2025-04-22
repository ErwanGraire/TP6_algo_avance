#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <chrono>
#include <limits>
#include <cmath>
#include <sstream>
#include <thread>
#include <mutex>
#include <cassert>
#include <cstdlib>

#if __cplusplus >= 201703L
#include <filesystem>
namespace fs = std::filesystem;
#endif

// Structures de données
struct Produit {
    int id;
    double poids;
    double valeur;
    double ratio;
};

struct Ville {
    int id;
    std::string nom;
};

class GestionDonnees {
public:
    std::vector<Produit> produits;
    double capaciteMax;
    std::vector<Ville> villes;
    std::vector<std::vector<double>> matriceEnergie;

    //Q2****
    bool lireProduits(const std::string& fichier) {
        std::ifstream fin(fichier);
        if (!fin) {
            std::cerr << "Erreur: Impossible d'ouvrir le fichier " << fichier << std::endl;
            return false;
        }

        std::string ligne;
        if (!std::getline(fin, ligne)) {
            std::cerr << "Erreur: Fichier vide ou format incorrect (ligne 1)" << std::endl;
            return false;
        }

        std::istringstream iss(ligne);
        int nbProduits;
        double capacite;
        char delim;
        if (!(iss >> nbProduits >> delim >> capacite) || delim != ';') {
            std::cerr << "Erreur: Format de fichier incorrect (ligne 1)" << std::endl;
            return false;
        }

        if (nbProduits <= 0 || capacite <= 0) {
            std::cerr << "Erreur: Nombre de produits ou capacité invalide (ligne 1)" << std::endl;
            return false;
        }

        capaciteMax = capacite;
        produits.clear();

        for (int i = 0; i < nbProduits; ++i) {
            if (!std::getline(fin, ligne)) {
                std::cerr << "Erreur: Nombre de lignes insuffisant pour les produits (attendu " << nbProduits << ")" << std::endl;
                return false;
            }

            std::istringstream issProduit(ligne);
            Produit p;
            if (!(issProduit >> p.id >> delim >> p.poids >> delim >> p.valeur) || delim != ';') {
                std::cerr << "Erreur: Format de fichier incorrect (ligne " << i + 2 << ")" << std::endl;
                return false;
            }

            if (p.poids <= 0 || p.valeur <= 0) {
                std::cerr << "Erreur: Poids ou valeur invalide pour le produit ID " << p.id << " (ligne " << i + 2 << ")" << std::endl;
                return false;
            }

            p.ratio = p.valeur / p.poids;
            produits.push_back(p);
        }

        return true;
    }

    bool lireVilles(const std::string& fichier) {
        std::ifstream fin(fichier);
        if (!fin) {
            std::cerr << "Erreur: Impossible d'ouvrir le fichier " << fichier << std::endl;
            return false;
        }

        std::string ligne;
        if (!std::getline(fin, ligne)) {
            std::cerr << "Erreur: Fichier vide ou format incorrect (ligne 1)" << std::endl;
            return false;
        }

        int nbVilles;
        try {
            nbVilles = std::stoi(ligne);
        }
        catch (...) {
            std::cerr << "Erreur: Format de fichier incorrect (ligne 1)" << std::endl;
            return false;
        }

        if (nbVilles <= 0) {
            std::cerr << "Erreur: Nombre de villes invalide (ligne 1)" << std::endl;
            return false;
        }

        villes.clear();
        for (int i = 0; i < nbVilles; ++i) {
            if (!std::getline(fin, ligne)) {
                std::cerr << "Erreur: Nombre de lignes insuffisant pour les villes (attendu " << nbVilles << ")" << std::endl;
                return false;
            }

            Ville v = { i, ligne };
            villes.push_back(v);
        }

        matriceEnergie.resize(nbVilles, std::vector<double>(nbVilles));
        for (int i = 0; i < nbVilles; ++i) {
            if (!std::getline(fin, ligne)) {
                std::cerr << "Erreur: Nombre de lignes insuffisant pour la matrice d'énergie (attendu " << nbVilles << ")" << std::endl;
                return false;
            }

            std::istringstream iss(ligne);
            for (int j = 0; j < nbVilles; ++j) {
                if (!(iss >> matriceEnergie[i][j])) {
                    std::cerr << "Erreur: Format de matrice incorrect (ligne " << nbVilles + i + 2 << ")" << std::endl;
                    return false;
                }

                if (matriceEnergie[i][j] < 0) {
                    std::cerr << "Erreur: Valeur négative dans la matrice d'énergie (ligne " << nbVilles + i + 2 << ", colonne " << j + 1 << ")" << std::endl;
                    return false;
                }
            }
        }

        return true;
    }
};

class AlgorithmesGloutonsRandomises {
private:
    GestionDonnees& donnees;
    std::mt19937 gen;
    std::mutex mtx;

    // Variables pour stocker les meilleures solutions
    std::vector<Produit> meilleureSolutionP1;
    std::vector<int> meilleureSolutionP2;
    double meilleurScoreP1 = -std::numeric_limits<double>::infinity();
    double meilleurScoreP2 = -std::numeric_limits<double>::infinity();

public:
    AlgorithmesGloutonsRandomises(GestionDonnees& d, unsigned int graine) : donnees(d), gen(graine) {}



    //Q5****
    std::vector<Produit> resoudreSacADosRandomise(int N = 2) {
        auto produitsTries = donnees.produits;
        std::sort(produitsTries.begin(), produitsTries.end(), [](const Produit& a, const Produit& b) {
            return a.ratio > b.ratio;
            });

        std::vector<Produit> solution;
        double capaciteUtilisee = 0.0;
        std::uniform_int_distribution<> dist(0, N - 1);

        for (size_t i = 0; i < produitsTries.size(); i += N) {
            std::vector<Produit> candidats;
            for (size_t j = i; j < i + N && j < produitsTries.size(); ++j) {
                if (capaciteUtilisee + produitsTries[j].poids <= donnees.capaciteMax) {
                    candidats.push_back(produitsTries[j]);
                }
            }
            if (!candidats.empty()) {
                int choix = dist(gen) % candidats.size();
                solution.push_back(candidats[choix]);
                capaciteUtilisee += candidats[choix].poids;
            }
        }
        return solution;
    }

    std::vector<int> resoudreTourneeRandomisee(int M = 2) {
        int nbVilles = donnees.villes.size();
        std::vector<bool> visite(nbVilles, false);
        std::vector<int> tournee;
        int villeActuelle = 0;
        tournee.push_back(villeActuelle);
        visite[villeActuelle] = true;
        std::uniform_int_distribution<> dist(0, M - 1);

        while (tournee.size() < nbVilles) {
            std::vector<std::pair<int, double>> distances;
            for (int v = 0; v < nbVilles; ++v) {
                if (!visite[v]) {
                    distances.emplace_back(v, donnees.matriceEnergie[villeActuelle][v]);
                }
            }
            std::sort(distances.begin(), distances.end(), [](const auto& a, const auto& b) {
                return a.second < b.second;
                });
            int nbCandidats = std::min(M, static_cast<int>(distances.size()));
            if (nbCandidats > 0) {
                int choix = dist(gen) % nbCandidats;
                int prochaineVille = distances[choix].first;
                tournee.push_back(prochaineVille);
                visite[prochaineVille] = true;
                villeActuelle = prochaineVille;
            }
        }
        return tournee;
    }


    //Q11***
    std::pair<double, double> calculerResultats(const std::vector<Produit>& sac, const std::vector<int>& tournee) {
        double benefice = 0.0;
        for (const auto& p : sac) benefice += p.valeur;

        double energie = 0.0;
        for (size_t i = 0; i < tournee.size() - 1; ++i) {
            energie += donnees.matriceEnergie[tournee[i]][tournee[i + 1]];
        }
        if (!tournee.empty()) {
            energie += donnees.matriceEnergie[tournee.back()][0];
        }
        return std::make_pair(benefice, energie);
    }


    //Q8****
    void executerReplication(unsigned int graine, const std::string& fichierProduits, const std::string& fichierVilles, std::ofstream& resultatsFichier) {
        GestionDonnees donneesLocale;
        if (!donneesLocale.lireProduits(fichierProduits) || !donneesLocale.lireVilles(fichierVilles)) {
            std::lock_guard<std::mutex> lock(mtx);
            std::cerr << "Erreur lors de la lecture des fichiers pour la graine " << graine << std::endl;
            return;
        }

        AlgorithmesGloutonsRandomises algo(donneesLocale, graine);
        auto sac = algo.resoudreSacADosRandomise();
        auto tournee = algo.resoudreTourneeRandomisee();
        auto resultats = algo.calculerResultats(sac, tournee);
        double benefice = resultats.first;
        double energie = resultats.second;

        double scoreP1 = benefice;
        double scoreP2 = benefice - energie;

        {
            std::lock_guard<std::mutex> lock(mtx);
            // Mettre à jour la meilleure solution pour P1
            if (scoreP1 > meilleurScoreP1) {
                meilleurScoreP1 = scoreP1;
                meilleureSolutionP1 = sac;
            }

            // Mettre à jour la meilleure solution pour P2
            if (scoreP2 > meilleurScoreP2) {
                meilleurScoreP2 = scoreP2;
                meilleureSolutionP2 = tournee;
            }

            resultatsFichier << graine << "," << benefice << "," << energie << "," << (benefice - energie) << std::endl;
        }
    }


    //Q4*****
    void afficherResultats() {
        // Résolution des deux problèmes
        auto sac = resoudreSacADosRandomise();
        auto tournee = resoudreTourneeRandomisee();

        // Calcul des résultats
        auto resultats = calculerResultats(sac, tournee);
        double benefice = resultats.first;
        double energie = resultats.second;

        // Affichage des résultats pour P1 (sac à dos)
        std::cout << "=== Résultats pour le problème P1 (Sac à dos) ===" << std::endl;
        std::cout << "Produits sélectionnés : " << std::endl;
        for (const auto& produit : sac) {
            std::cout << "  - ID: " << produit.id
                << ", Poids: " << produit.poids
                << ", Valeur: " << produit.valeur
                << ", Ratio: " << produit.ratio << std::endl;
        }

        // Affichage des résultats pour P2 (tournée)
        std::cout << "=== Résultats pour le problème P2 (Tournée) ===" << std::endl;
        std::cout << "Villes visitées dans l'ordre : " << std::endl;
        for (int villeId : tournee) {
            std::cout << "  - Ville ID: " << villeId
                << " (" << donnees.villes[villeId].nom << ")" << std::endl;
        }

        // Affichage des bénéfices et de l'énergie consommée
        std::cout << "=== Résultats globaux ===" << std::endl;
        std::cout << "Bénéfice total : " << benefice << std::endl;
        std::cout << "Énergie totale consommée : " << energie << std::endl;
        std::cout << "Score (Bénéfice - Énergie) : " << (benefice - energie) << std::endl;
    }

    // Accesseurs pour les meilleures solutions
    const std::vector<Produit>& getMeilleureSolutionP1() const { return meilleureSolutionP1; }
    const std::vector<int>& getMeilleureSolutionP2() const { return meilleureSolutionP2; }
    double getMeilleurScoreP1() const { return meilleurScoreP1; }
    double getMeilleurScoreP2() const { return meilleurScoreP2; }
};



//Q8***
void genererScriptLinux(int nbReplications, const std::string& nomProgramme, const std::string& fichierProduits, const std::string& fichierVilles) {
    std::ofstream script("replications.sh");
    script << "#!/bin/bash\n";
    for (int i = 1; i <= nbReplications; ++i) {
        script << "./" << nomProgramme << " " << i << " " << fichierProduits << " " << fichierVilles << " >> resultats.txt\n";
    }
    script.close();
#if __cplusplus >= 201703L
    fs::permissions("replications.sh", fs::perms::owner_exec, fs::perm_options::add);
#else
    std::system("chmod +x replications.sh");
#endif
}

void genererScriptWindows(int nbReplications, const std::string& nomProgramme, const std::string& fichierProduits, const std::string& fichierVilles) {
    std::ofstream script("replications.bat");
    for (int i = 1; i <= nbReplications; ++i) {
        script << "start /B " << nomProgramme << ".exe " << i << " " << fichierProduits << " " << fichierVilles << " >> resultats.txt\n";
    }
    script.close();
}


//Q10*** 
void genererFichierProduits(const std::string& fichier) {
    std::ofstream fout(fichier);
    fout << "5;30.0\n";
    fout << "1;6.0;3.0\n";
    fout << "2;10.0;4.0\n";
    fout << "3;11.0;4.0\n";
    fout << "4;15.0;6.0\n";
    fout << "5;8.0;3.0\n";
    fout.close();
}

void genererFichierVilles(const std::string& fichier) {
    std::ofstream fout(fichier);
    fout << "4\n";
    fout << "Lille\n";
    fout << "Arros\n";
    fout << "Amiens\n";
    fout << "Loos\n";
    fout << "0.0 10.0 15.0 20.0\n";
    fout << "10.0 0.0 12.0 18.0\n";
    fout << "15.0 12.0 0.0 8.0\n";
    fout << "20.0 18.0 8.0 0.0\n";
    fout.close();
}

void testLectureFichiers() {
    genererFichierProduits("test_produits.txt");
    genererFichierVilles("test_villes.txt");

    GestionDonnees donnees;
    assert(donnees.lireProduits("test_produits.txt"));
    assert(donnees.lireVilles("test_villes.txt"));
    assert(donnees.produits.size() == 5);
    assert(donnees.villes.size() == 4);
    assert(donnees.matriceEnergie.size() == 4);
    assert(donnees.matriceEnergie[0].size() == 4);
}

void testAlgorithmesGloutonsRandomises() {
    genererFichierProduits("test_produits.txt");
    genererFichierVilles("test_villes.txt");

    GestionDonnees donnees;
    donnees.lireProduits("test_produits.txt");
    donnees.lireVilles("test_villes.txt");

    AlgorithmesGloutonsRandomises algo(donnees, 42);
    auto sac = algo.resoudreSacADosRandomise();
    auto tournee = algo.resoudreTourneeRandomisee();
    auto resultats = algo.calculerResultats(sac, tournee);
    double benefice = resultats.first;
    double energie = resultats.second;

    assert(benefice > 0);
    assert(energie > 0);
}

void testReplications() {
    genererFichierProduits("test_produits.txt");
    genererFichierVilles("test_villes.txt");

    GestionDonnees donnees;
    std::ofstream resultatsFichier("test_resultats.txt");
    AlgorithmesGloutonsRandomises algo(donnees, 42);
    algo.executerReplication(1, "test_produits.txt", "test_villes.txt", resultatsFichier);
    algo.executerReplication(2, "test_produits.txt", "test_villes.txt", resultatsFichier);
    resultatsFichier.close();

    std::ifstream fin("test_resultats.txt");
    std::string ligne;
    int count = 0;
    while (std::getline(fin, ligne)) {
        ++count;
    }
    assert(count == 2);
}


//Q10**
void genererGrandFichierProduits(const std::string& fichier, int nbProduits) {
    std::ofstream fout(fichier);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> poidsDist(1.0, 20.0);
    std::uniform_real_distribution<> valeurDist(1.0, 50.0);

    fout << nbProduits << ";1000.0\n";  // Capacité fixée à 1000
    for (int i = 1; i <= nbProduits; ++i) {
        double poids = poidsDist(gen);
        double valeur = valeurDist(gen);
        fout << i << ";" << poids << ";" << valeur << "\n";
    }
    fout.close();
}

void genererGrandFichierVilles(const std::string& fichier, int nbVilles) {
    std::ofstream fout(fichier);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dist(5.0, 200.0);

    fout << nbVilles << "\n";
    for (int i = 0; i < nbVilles; ++i) {
        fout << "Ville_" << i << "\n";
    }

    for (int i = 0; i < nbVilles; ++i) {
        for (int j = 0; j < nbVilles; ++j) {
            double distance = (i == j) ? 0.0 : dist(gen);
            fout << distance << " ";
        }
        fout << "\n";
    }
    fout.close();
}


//Q12****
void benchmark(int nbReplications, int nbProduits, int nbVilles) {
    // Génération des fichiers
    genererGrandFichierProduits("bench_produits.txt", nbProduits);
    genererGrandFichierVilles("bench_villes.txt", nbVilles);

    GestionDonnees donnees;
    donnees.lireProduits("bench_produits.txt");
    donnees.lireVilles("bench_villes.txt");

    // Mesure du temps
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < nbReplications; ++i) {
        AlgorithmesGloutonsRandomises algo(donnees, i);
        auto sac = algo.resoudreSacADosRandomise();
        auto tournee = algo.resoudreTourneeRandomisee();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Benchmark pour " << nbProduits << " produits et " << nbVilles << " villes:\n";
    std::cout << "Temps total pour " << nbReplications << " réplications: " << duration.count() << " ms\n";
    std::cout << "Temps moyen par réplication: " << duration.count() / nbReplications << " ms\n";
}

void construireNouvelleSolution(const AlgorithmesGloutonsRandomises& algo, const GestionDonnees& donnees) {
    std::cout << "=== Nouvelle Solution ===" << std::endl;

    // Afficher la meilleure solution pour P1
    std::cout << "Meilleure solution pour P1 (Sac à dos) : " << std::endl;
    for (const auto& produit : algo.getMeilleureSolutionP1()) {
        std::cout << "  - ID: " << produit.id
            << ", Poids: " << produit.poids
            << ", Valeur: " << produit.valeur
            << ", Ratio: " << produit.ratio << std::endl;
    }

    // Afficher la meilleure solution pour P2
    std::cout << "Meilleure solution pour P2 (Tournée) : " << std::endl;
    for (int villeId : algo.getMeilleureSolutionP2()) {
        std::cout << "  - Ville ID: " << villeId
            << " (" << donnees.villes[villeId].nom << ")" << std::endl;
    }

    // Afficher les scores
    std::cout << "Score P1 : " << algo.getMeilleurScoreP1() << std::endl;
    std::cout << "Score P2 : " << algo.getMeilleurScoreP2() << std::endl;
}

int main(int argc, char* argv[]) {
    // Tests
    testLectureFichiers();
    testAlgorithmesGloutonsRandomises();
    testReplications();

    if (argc < 3 || argc > 4) {
        std::cerr << "Usage: " << argv[0] << " <fichier_produits> <fichier_villes> [graine]" << std::endl;
        std::cerr << "Ou pour générer un script: " << argv[0] << " script <nb_replications> <fichier_produits> <fichier_villes>" << std::endl;
        return 1;
    }


    //Q9***
    if (std::string(argv[1]) == "script") {
        if (argc != 5) {
            std::cerr << "Usage: " << argv[0] << " script <nb_replications> <fichier_produits> <fichier_villes>" << std::endl;
            return 1;
        }
        int nbReplications = std::stoi(argv[2]);
        std::string fichierProduits = argv[3];
        std::string fichierVilles = argv[4];
        genererScriptLinux(nbReplications, argv[0], fichierProduits, fichierVilles);
        genererScriptWindows(nbReplications, argv[0], fichierProduits, fichierVilles);
        std::cout << "Scripts générés : replications.sh (Linux) et replications.bat (Windows)" << std::endl;
        return 0;
    }

    std::string fichierProduits = argv[1];
    std::string fichierVilles = argv[2];
    unsigned int graine = (argc == 4) ? static_cast<unsigned int>(std::stoul(argv[3])) : std::random_device{}();

    GestionDonnees donnees;
    if (!donnees.lireProduits(fichierProduits) || !donnees.lireVilles(fichierVilles)) {
        return 1;
    }

    AlgorithmesGloutonsRandomises algo(donnees, graine);
    auto sac = algo.resoudreSacADosRandomise();
    auto tournee = algo.resoudreTourneeRandomisee();
    auto resultats = algo.calculerResultats(sac, tournee);
    double benefice = resultats.first;
    double energie = resultats.second;

    std::cout << "=== Résultats pour la graine " << graine << " ===" << std::endl;
    std::cout << "Bénéfice total : " << benefice << std::endl;
    std::cout << "Énergie totale : " << energie << std::endl;
    std::cout << "Score (Bénéfice - Énergie) : " << (benefice - energie) << std::endl;

    if (argc == 3) {
        std::ofstream resultatsFichier("resultats.txt", std::ios::app);
        resultatsFichier << graine << "," << benefice << "," << energie << "," << (benefice - energie) << std::endl;
        resultatsFichier.close();
    }

    return 0;
}
