#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RED "[0;31m"
#define YLW "[0;33m"
#define RST "[0m"

struct ruter {
  unsigned int ruter_id;
  unsigned char flag;
  char* modell_navn;
  short ant_koblinger;
  unsigned char* koblinger;
};

int n;
struct ruter** rutere;

// Henter ruter-instansen ut if fra gitt id,
// hvis ikke finst print feilmeld og returner NULL
// param: unsigned int ruter_id: id til ruter som skal hentes
struct ruter* hent_ruter(unsigned int ruter_id) {
  int i;
  for(i = 0; i < n; i++) {
    if(rutere[i]->ruter_id == ruter_id) {
      return rutere[i];
    }
  }
  printf("ERROR: Fant ingen ruter m/ id %d\n", ruter_id);
  return NULL;
}


// KOMMANDOER
// Har samme funksjoner som i obligteksten

// param: unsigned int ruter_id: id til ruter som skal skrives ut info om
void print(unsigned int id) {
  int i;
  struct ruter* r = hent_ruter(id);
  if(!r) { return; }
  printf("%u: %s\n", r->ruter_id, r->modell_navn);
  printf("  flag: %u\n", r->flag);
  printf("    > aktiv       %u\n", (r->flag >> 0) & 1);
  printf("    > traadlos    %u\n", (r->flag >> 1) & 1);
  printf("    > 5GHz        %u\n", (r->flag >> 2) & 1);
  printf("    > endringsnr  %u\n", (r->flag >> 4) & 1111);
  if(r->ant_koblinger) {
    printf("  peker til:\n");
    for(i = 0; i < r->ant_koblinger; i++) {
      printf("    > %u\n", r->koblinger[i]);
    }
  }
}


// Sjekker om innflag og innverdi er valide, hvis ikke skriv feilmeld
// param: unsigned int ruter_id: id til ruter som skal endres
// param: unsigned char flag: flag som skal settes i ruter
// param: unsigned char verdi: verdi aa sette flag til
void sett_flag(unsigned int id, unsigned char flag, unsigned char verdi) {
  struct ruter* r = hent_ruter(id);
  if(!r) { return; }

  if((flag >= 0 && flag <= 2) || flag == 4) {
    if(flag < 3 && (verdi < 0 || verdi > 1)) {
      printf("ERROR: Ulovlig verdi for flag %u: %u\n", flag, verdi);
      printf("  > Lovlige verdier for flags [0, 1, 2]: [0, 1]\n");
      return;
    }
    if(flag == 4 && (verdi < 0 || verdi > 15)) {
      printf("ERROR: Ulovlig verdi for flag %u: %u\n", flag, verdi);
      printf("  > Lovlige verdier for flag %u: {0, 15}\n", flag);
      return;
    }

    // sett bitverdi
    if(verdi) {
      r->flag |= (1u << flag);
    } else {
      r->flag &= ~(1u << flag);
    }

  } else {
    printf("ERROR: Ulovlig flag: %u\n", flag);
    printf("  > Lovlige flags: [0, 1, 2, 4]\n");
    return;
  }
}


// param: unsigned int id: id til ruter som skal endres
// param: char* navn: det nye modellnavnet
void sett_modell(unsigned int id, char* navn) {
  int i, l = strlen(navn);
  struct ruter* r = hent_ruter(id);
  if(!r) { return; }
  // free: allokert i 'rutere_oppsett'
  free(r->modell_navn);
  // alooc: lengden av modell_navn + 1 (hvert char er 1 byte)
  r->modell_navn = calloc(sizeof(char), l + 1);
  if(!r->modell_navn) {
    printf("\033%sFATAL ERROR\033%s: Klarte ikke aa allokere minne.\n", RED, RST);
    exit(EXIT_FAILURE);
  }
  for(i = 0; i < l; i++) {
    r->modell_navn[i] = navn[i];
  } r->modell_navn[l] = '\0';
}


// param: unsigned int id_from: id koblingen skal peke fra
// param: unsigned int id_to: id koblingen skal peke til
void legg_til_kobling(unsigned int id_from, unsigned int id_to) {
  int i;
  struct ruter* r = hent_ruter(id_from);
  if(!r) { return; }
  //sjekk om kobling finst fra foer, peker til seg selv eller maks naadd
  if (r->ant_koblinger == 10) { return; }
  for(i = 0; i < r->ant_koblinger; i++) {
    if(r->koblinger[i] == id_from) { return; }
    if(r->koblinger[i] == id_to) { return; }
  }
  r->koblinger[r->ant_koblinger] = id_to;
  r->ant_koblinger++;
}


// Sletter foerst alle koblinger til ruter, saa ruteren selv.
// Kopierer data som ikke skal slettes fra gammel ruter array,
// setter inn i en array og frigjoer gammel array.
// param: unsigned int id: id til ruter som skal slettes
void slett_ruter(unsigned int id) {
  int i, j;
  struct ruter* r = hent_ruter(id);
  if(!r) { return; }

  // slett koblinger til
  for(i = 0; i < n; i++) {
    if(rutere[i]->ruter_id == id) { continue; }
    for(j = 0; j < rutere[i]->ant_koblinger; j++) {
      // hvis fant kobling til...
      if(rutere[i]->koblinger[j] == id) {
        for(j = j; j < rutere[i]->ant_koblinger; j++) {
          // skyv koblinger bak et hakk fram
          if(j != 9) {
            rutere[i]->koblinger[j] = rutere[i]->koblinger[j + 1];
          } else {
            rutere[i]->koblinger[j] = 0;
          }
        }
        rutere[i]->ant_koblinger--;
        break; //antar at ikke finst mer enn en identisk kobling - implementert i legg_til_kobling
      }
    }
  }

  // frigjoer allokert minne for gammel ruter array og kopier data til ny array
  // alloc: bytes er antall rutere * 8 (bytestoerrelse av pointer)
  struct ruter** ny_rutere = calloc(sizeof(struct ruter*), n - 1);
  if(!ny_rutere) {
    printf("\033%sFATAL ERROR\033%s: Klarte ikke aa allokere minne.\n", RED, RST);
    exit(EXIT_FAILURE);
  }
  for(i = 0, j = 0; i < n - 1; i++, j++) {
    // hopp over ruter som skal slettes
    if(rutere[j]->ruter_id == id) {
      j++;
     }
    // alloc: 32 bytes (bytestoerrelse for struct ruter)
    ny_rutere[i] = calloc(sizeof(struct ruter), 1);
    if(!ny_rutere[i]) {
      printf("\033%sFATAL ERROR\033%s: Klarte ikke aa allokere minne.\n", RED, RST);
      exit(EXIT_FAILURE);
    }
    memcpy(ny_rutere[i], rutere[j], sizeof(struct ruter));
    // free: allokert i 'rutere_oppsett'
    free(rutere[j]);
  } free(rutere);
  rutere = ny_rutere;
  n--;
  // frigjoer ruteren selv
  // free: allokert i 'rutere_oppsett'
  free(r->modell_navn);
  free(r->koblinger);
  free(r);
}


// Kaller paa hjelpefunksjon DFS,
// skriver ut om rute finnes eller ikke
// param: unsigned int id_from: id til ruter som ruten skal gaa fra
// param: unsigned int id_to: id til ruter som ruten skal gaa til
int finnes_rute(unsigned int id_from, unsigned int id_to) {
  int i;
  // alloc: n bytes
  unsigned char* routers_visited = calloc(sizeof(unsigned char), n);
  if(!routers_visited) {
    printf("\033%sFATAL ERROR\033%s: Klarte ikke aa allokere minne.\n", RED, RST);
    exit(EXIT_FAILURE);
  }

  //Kall til DFS
  i = DFS(id_from, id_to, routers_visited, NULL, 0);

  // free: allokert oeverst i funksjonen
  free(routers_visited);

  // print resultatet - synlighet
  if(i) {
    printf("Finnes rute fra %d til %d. Se over.\n", id_from, id_to);
  } else {
    printf("Finnes ikke rute fra %d til %d.\n", id_from, id_to);
  }
}


// Depth-first search - rekursiv funksjon
// return: (int bool) 1 om rute finnes, ellers 0
// Skriver ut sti om rute finnes
// param: unsigned int from: id til ruter som ruten skal gaa fra
// param: unsigned int to: id til ruter som ruten skal gaa til
// param: unsigned char* visited: liste over hvilke rutere som er besoekte
// param: unsigned int* path: liste over sti til naa
// param: int path_l: lengden av sti til naa
int DFS(unsigned int from, unsigned int to, unsigned char* visited, unsigned int* path, int path_l) {
  int i, j, l;
  struct ruter* r = hent_ruter(from);
  struct ruter* next;
  unsigned int* new_path;

  // Forbered ny stiskildring
  // alloc: 8 * (stilengde til naa + 1)
  new_path = calloc(sizeof(unsigned int), path_l + 1);
  for(i = 0; i < path_l; i++) {
    new_path[i] = path[i];
  }
  new_path[path_l] = from;


  // BASE CASE: fra erlik til - rute funnet
  if(from == to)  {
    // skriv ut sti
    printf("Rute: ");
    for(i = 0; i < path_l; i++) {
      printf("%d -> ", new_path[i]);
    } printf("%d\n", new_path[path_l]);

    // free: allokert over i dette DFS-kallet, allokert i DFS-kallet som kalte dette DFS-kallet
    free(new_path);
    free(path);
    return 1;
  }

  // Merk ruter som besoekt
  // (visited har samme plassering som rutere - ikke noedvendigvis lik ruter_id som indekstall)
  for(i = 0; i < n; i++) {
    if(rutere[i]->ruter_id == from) {
      visited[i] = 1;
      break;
    }
  }

  // for hver kobling...
  for(i = 0; i < r->ant_koblinger; i++) {
    next = hent_ruter(r->koblinger[i]);
    for(j = 0; j < n; j++) {
      // hvor koblingen ikke er besoekt fra foer...
      if(rutere[j]->ruter_id == next->ruter_id && !visited[j]) {
        // RECURSIVE CASE: finnes kobling som ikke er besoekt
        if(DFS(next->ruter_id, to, visited, new_path, path_l + 1) == 1) {
          // free: allokert i DFS-kallet som kalte dette DFS-kallet
          free(path);
          return 1;
        }
      }
    }
  }

  // free: allokert over i dette DFS-kallet
  free(new_path);

  // BASE CASE: ingen rute fra rekursivt kall - finnes ingen rute
  return 0;
}


// Oppretter array og rutere ut i fra innfil
// param: FILE* fp: peker til innfil
void rutere_oppsett(FILE* fp) {
  int i;
  unsigned int u_i;
  unsigned char c;

  fread(&n, sizeof(int), 1, fp);

  // alloc: n * 8 bytes (pekere)
  rutere = calloc(sizeof(struct ruter*), n);
  if(!rutere) {
    printf("\033%sFATAL ERROR\033%s: Klarte ikke aa allokere minne.\n", RED, RST);
    exit(EXIT_FAILURE);
  }

  // for hver linje med ruter info,
  // opprett ruterinstans og legg inn data
  for(i = 0; i < n; i++) {
    int j;
    // alloc: 32 bytes
    rutere[i] = calloc(sizeof(struct ruter), 1);
    if(!rutere[i]) {
      printf("\033%sFATAL ERROR\033%s: Klarte ikke aa allokere minne.\n", RED, RST);
      exit(EXIT_FAILURE);
    }
    // alloc: 4 * 10 bytes
    rutere[i]->koblinger = calloc(sizeof(unsigned int), 10);
    if(!rutere[i]->koblinger) {
      printf("\033%sFATAL ERROR\033%s: Klarte ikke aa allokere minne.\n", RED, RST);
      exit(EXIT_FAILURE);
    }
    fread(&rutere[i]->ruter_id, sizeof(unsigned int), 1, fp);
    fread(&rutere[i]->flag, sizeof(unsigned char), 1, fp);
    fread(&c, sizeof(unsigned char), 1, fp);
    // alloc: navnlengde + 1
    rutere[i]->modell_navn = calloc(sizeof(char), c + 1);
    if(!rutere[i]->modell_navn) {
      printf("\033%sFATAL ERROR\033%s: Klarte ikke aa allokere minne.\n", RED, RST);
      exit(EXIT_FAILURE);
    }
    for(j = 0; j < c; j++) {
      fread(&rutere[i]->modell_navn[j], sizeof(char), 1, fp);
    } rutere[i]->modell_navn[c] = '\0'; //LEGAL?
    rutere[i]->ant_koblinger = 0;

    fread(&c, sizeof(unsigned char), 1, fp);

    // hvis sluttbit ikke er 0...
    if(c) {
      printf("\033%sFATAL ERROR\033%s: Inndata er feilformatert.\n", RED, RST);
      printf("> Forventet '0' for å avslutte informasjonsblokken, men leste '%u'.\n", c);
      exit(EXIT_FAILURE);
    }
  }

  //legg til koblinger
  while(!feof(fp)) {
    unsigned int fra, til;
    struct ruter* r_fra;
    struct ruter* r_til;

    fread(&fra, sizeof(unsigned int), 1, fp);
    fread(&til, sizeof(unsigned int), 1, fp);

    r_fra = hent_ruter(fra);
    r_til = hent_ruter(til);
    if(!r_fra || !r_til) {
      printf("ERROR: Inndata er feilformatert.\n");
      printf("En eller begge rutere med gitte id-er finnes ikke: %d, %d\n", fra, til);
    } else {
      legg_til_kobling(fra, til);
    }

    fread(&c, sizeof(unsigned char), 1, fp);

    // hvis sluttbit ikke er 0...
    if(c) {
      printf("\033%sFATAL ERROR\033%s: Inndata er feilformatert.\n", RED, RST);
      printf("> Forventet '0' for å avslutte informasjonsblokken, men leste '%u'.\n", c);
      exit(EXIT_FAILURE);
    }
  }

}


// Hjelpemetode for 'les_kommandoer',
// henter neste data delt av mellomrom og linjeskift
// param: FILE* fp: peker til innfil
char* get_next_input(FILE* fp) {
  int i = 0, l;
  char c;
  char buffer[248];
  char* str;

  memset(buffer, 0, 248);

  fread(&c, 1, 1, fp);

  // les neste inndata
  while(isprint(c) && c!= 32) {
    buffer[i] = c;
    fread(&c, 1, 1, fp);
    i++;
  }

  l = i;
  // alloc - lengde av inndatastring + 1
  str = calloc(sizeof(char), (1 + l));
  if(!str) {
    printf("\033%sFATAL ERROR\033%s: Klarte ikke aa allokere minne.\n", RED, RST);
    exit(EXIT_FAILURE);
  }

  // legg data fra buffer inn i dynamisk allokert minne
  for(i = 0; i < l; i++) {
    str[i] = buffer[i];
  } str[i] = '\0';

  return str;
}


// Utfoerer kommanoer ut fra innfil
// param: FILE* fp: peker til innfil
void les_kommandoer(FILE* fp) {
  char buffer[200];
  char c;
  int i = 0;

  memset(buffer, 0, 200);
  fread(&c, 1, 1, fp);

  while(!feof(fp)) {
    // les kommando inn i buffer fra innfil
    while(isprint(c) && c != 32) {
      buffer[i] = c;
      fread(&c, 1, 1, fp);
      i++;
    }

    // sammenlign med kjente kommandoer
    // les resten av params og utfoer

    if(strcmp(buffer, "print\0") == 0) {
      unsigned int input_id;
      char* str;

      str = get_next_input(fp);
      input_id = atoi(str);

      print(input_id);

      // free: allokert i get_next_input
      free(str);

    } else if(strcmp(buffer, "sett_flag\0") == 0) {
      unsigned int input_id;
      unsigned char input_flag, input_verdi;
      char* str1;
      char* str2;
      char* str3;

      str1 = get_next_input(fp);
      input_id = atoi(str1);
      str2 = get_next_input(fp);
      input_flag = (unsigned char)atoi(str2);
      str3 = get_next_input(fp);
      input_verdi = (unsigned char)atoi(str3);

      sett_flag(input_id, input_flag, input_verdi);

      // free: allokert i get_next_input
      free(str1); free(str2); free(str3);

    } else if(strcmp(buffer, "sett_modell\0") == 0) {
      unsigned int input_id;
      char* input_modell_navn;
      char name_buffer[248];
      char c;
      int i = 0, l;
      char* str;

      str = get_next_input(fp);
      input_id = atoi(str);
      memset(name_buffer, ' ', 248);
      fread(&c, 1, 1, fp);
      while(c != '\n') {
        name_buffer[i] = c;
        fread(&c, 1, 1, fp);
        i++;
      }
      l = i;
      // alloc: lengden av navn  + 1
      input_modell_navn = calloc(sizeof(char), l + 1);
      if(!input_modell_navn) {
        printf("\033%sFATAL ERROR\033%s: Klarte ikke aa allokere minne.\n", RED, RST);
        exit(EXIT_FAILURE);
      }
      for(i = 0; i < l; i++) {
        input_modell_navn[i] = name_buffer[i];
      } input_modell_navn[l] = '\0';

      sett_modell(input_id, input_modell_navn);

      // free: allokert i get_next_input, allokert rett ovenfor
      free(str); free(input_modell_navn);

    } else if(strcmp(buffer, "legg_til_kobling\0") == 0) {
      unsigned int input_id_from, input_id_to;
      char* str1;
      char* str2;

      str1 = get_next_input(fp);
      input_id_from = atoi(str1);
      str2 = get_next_input(fp);
      input_id_to = atoi(str2);

      legg_til_kobling(input_id_from, input_id_to);

      // free: allokert i get_next_input
      free(str1); free(str2);

    // NB! Forskjellig i 10_routers_10_edges og obligtekst
    } else if(strcmp(buffer, "slett_router\0") == 0 || strcmp(buffer, "slett_ruter\0") == 0) {
      unsigned int input_id;
      char* str;

      str = get_next_input(fp);
      input_id = atoi(str);

      slett_ruter(input_id);

      // free: allokert i get_next_input
      free(str);

    } else if(strcmp(buffer, "finnes_rute\0") == 0) {
      unsigned int input_id_from, input_id_to;
      char* str1;
      char* str2;

      str1 = get_next_input(fp);
      input_id_from = atoi(str1);
      str2 = get_next_input(fp);
      input_id_to = atoi(str2);

      finnes_rute(input_id_from, input_id_to);

      // free: allokert i get_next_input
      free(str1); free(str2);

    } else {
      printf("ERROR: Ukjent kommando innfil.\n");
      printf("  > %s\n", buffer);
    }

    i = 0;
    memset(buffer, 0, 200);
    fread(&c, 1, 1, fp); //linjeskift
  }
}

// Skriver ruterdata til fil
// param: FILE* fp: peker til utfil
void skriv_til_fil(FILE* fp) {
  int i, j;
  unsigned char end = 0;

  fwrite(&n, sizeof(int), 1, fp);

  // for hver ruter, skriv data
  for(i = 0; i < n; i++) {
    struct ruter* r = rutere[i];
    unsigned char name_len = (unsigned char)strlen(r->modell_navn);
    fwrite(&r->ruter_id, sizeof(unsigned int), 1, fp);
    fwrite(&r->flag, sizeof(unsigned char), 1, fp);
    fwrite(&name_len, sizeof(unsigned char), 1, fp);
    for(j = 0; r->modell_navn[j] != '\0'; j++) {
      fwrite(&r->modell_navn[j], sizeof(char), 1, fp);
    } fwrite(&end, sizeof(unsigned char), 1, fp);
  }

  // for hver kobling, skriv ruter id fra og til
  for(i = 0; i < n; i++) {
    struct ruter* r = rutere[i];
    for(j = 0; j < r->ant_koblinger; j++) {
      fwrite(&r->ruter_id, sizeof(unsigned int), 1, fp);
      fwrite(&r->koblinger[j], sizeof(unsigned int), 1, fp);
      fwrite(&end, sizeof(unsigned char), 1, fp);
    }
  }
}


// Frigjoer minnet allokert til ruterene
void rutere_frigjoer() {
  int i;
  // free: initielt allokert i 'rutere_oppsett'
  for(i = 0; i < n; i++) {
    free(rutere[i]->modell_navn);
    free(rutere[i]->koblinger);
    free(rutere[i]);
  } free(rutere);
}


int main(int argc, char** argv) {
  int i;
  FILE* fp_ruter;
  FILE* fp_kommando;

  if(argc != 3) {
    printf("\033%sFATAL ERROR\033%s: Feil antall argumenter.\n", RED, RST);
    printf("'./ruterdrift [ruterfil] [kommmandofil]'\n");
    return EXIT_FAILURE;
  }

  // Les ruterfil
  fp_ruter = fopen(argv[1], "rb");
  if(!fp_ruter) {
    printf("\033%sFATAL ERROR\033%s: Klarte ikke aapne innfil m/ruterinfo.\n", RED, RST);
    return EXIT_FAILURE;
  }
  rutere_oppsett(fp_ruter);
  fclose(fp_ruter);

  // Les kommandofil
  fp_kommando = fopen(argv[2], "r");
  if(!fp_kommando) {
    printf("\033%sFATAL ERROR\033%s: Klarte ikke aapne innfil m/kommandoer.\n", RED, RST);
    return EXIT_FAILURE;
  }
  les_kommandoer(fp_kommando);
  fclose(fp_kommando);

  // Skriv til ruterfil
  fp_ruter = fopen(argv[1], "wb");
  if(!fp_ruter) {
    printf("\033%sFATAL ERROR\033%s: Klarte ikke aapne utfil.\n", RED, RST);
    return EXIT_FAILURE;
  }
  skriv_til_fil(fp_ruter);
  fclose(fp_ruter);

  // Frigjoer minnet
  rutere_frigjoer();

  // OK
  return EXIT_SUCCESS;
}
