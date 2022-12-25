#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#pragma pack(1)
union BMPHeader{

    char buffer[14];

    struct {
        char ID[2];       // Сигнатура "BM"
        int size;           //  Размер файла
        char reserved1[2]; //  Зарезервировано
        char reserved2[2]; //  Зарезервировано
        int offset;     //  Смещение изображения от начала файла
    }data;

}BMPHeader;

union DIBHeader{

    char buffer[40];

    struct {
        int sizeofDIBHeader;          // Длина заголовка
        int width;           // Ширина изображения
        int height;          // Высота изображения
        short planes;        // Число плоскостей
        short bitsperpix;      // Глубина цвета
        int compression;     // Тип компрессии
        int imagesizewithpadding;       // Размер изображения, байт
        int XPIXPERMET;   // Горизонтальное разрешение
        int YPIXPERMET;   // Вертикальное разрешение
        int colors;      // Число используемых цветов
        int importantcolors; // Число основных цветов
    }data;

}DIBHeader;
#pragma pop;

struct parametersofimage{
    int width; // ширина 
    int height; // высота
    int size; // размер
};

void game_of_life(int **life, int h, int w) {
    
    int tmp[h][w]; // вспомогательный массив
    int countneighbours; // кол-во соседей
    

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            tmp[y][x] = life[y][x];
        }
    }

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            countneighbours = 0;

            // проверяем кол-во соседей
            for (int i = -1; i < 2; i++) {
                for (int j = -1; j < 2; j++) {
                    if ( (y+i > -1) && (x+j > -1) && (i != 0 || j != 0)) {
                        countneighbours += life[(y + i + h) % h][(x + j + w) % w];
                    }
                }
            }
            
            if (countneighbours == 3) { // если соседа 3 клетка зараждается или прожолжает жить
                tmp[y][x] = 1;

            // если соседа 2 клетка так и остается жить

            } else if (countneighbours < 2 || countneighbours > 3) { // если соседа меньше 2 или больше 3, клетка умирает или продолжает быть 0
                tmp[y][x] = 0;
            }
        }
    }
    int newval = 0; // переменная для проверки на изменение картинки
    // переписываем новую жизнь в основной массив
    for (int y = 0; y < h; y++){
        for (int x = 0; x < w; x++) {
            if (life[y][x] != tmp[y][x]) newval++;
            life[y][x] = tmp[y][x];
        }}
    if (newval == 0) exit(0); // если картинка не поменялась - крашим программу

}



int main(int argc, char* argv[]) {
    FILE * file; // для чтения файла
    char* filename; // название подаваемого файла
    char* directoryname;  // название директории, куда сохранять 
    int max_iter = INT32_MAX; // максимальное число поколений
    int dump_freq = 1; //Частота с которой программа должна сохранять поколения виде картинки.

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--input")) { 
            filename = argv[i + 1]; // считываем имя файла
        }
        else if (!strcmp(argv[i], "--output")) {
            directoryname = argv[i + 1]; // считываем имя дериктории
            mkdir(directoryname,0700); //создаем папку
        }
        else if (!strcmp(argv[i], "--max_iter")) {
            max_iter = strtol(argv[i + 1], 0, 10); // переводим строку в int и сохраняем кол-во итераций
        }
        else if (!strcmp(argv[i], "--dump_freq")) {
            dump_freq = strtol(argv[i + 1], 0, 10); // переводим строку в int и сохраняем частоту сохранений

        }
    }

    file = fopen(filename,"rb"); // Открываем двоичный файл для чтения
    if (!file){ // если он не читается, выводим ошибку
        printf("File can't be open");
        return 0;
    }
    
    // заполняем заголовки
    union BMPHeader bmph;
    fread(bmph.buffer, 1, 14, file);
    union DIBHeader diph;
    fread(diph.buffer, 1, 40, file);

    // сохраняем нужные параметры
    struct parametersofimage image;
    image.width = diph.data.width;
    image.height = diph.data.height;
    image.size = bmph.data.size - 54;

    // создаем массив для изображения
    int **img = (int**)malloc(image.height * sizeof(int*));
    for (int i = 0; i < image.height; i++) {
        img[i] = (int*)malloc(image.width * sizeof(int));
    }

    int padding = (4 - (image.width * 3) % 4) % 4; // расчитываем выравнивание 

    unsigned char imgbytes[image.size];
    fread(imgbytes, 1, image.size, file); // считываем само изображение

    int it = 0;
    // переписываем картинку в виде 0 и 1 
    for (int i = 0; i < image.height; i++) {
        for (int j = 0; j < image.width; j++) {
                img[i][j] = (imgbytes[it] != 255 || imgbytes[it+1] != 255 || imgbytes[it+2] != 255);
                it+=3;
        }
        it += padding; // прибавляем выравнивание
    }
    fclose(file); // закрываем считанный файл

    int iter = 1; // текущая жизнь
    while(iter <= max_iter){ // проходим все жизни

        if (iter % dump_freq == 0){ // если нужно сохраниться

            char path[1024] = {0}; 
            sprintf(path, "%s/%d%s", directoryname, iter,".bmp"); // соединяем название пути

            FILE* life = fopen(path, "w"); // открываем файл, в который будем записывать жизнь
            fwrite(bmph.buffer, 1, 14, life); // записываем BMPHeader в новый файл
            fwrite(diph.buffer, 1, 40, life); // записываем DIBHeader в новый файл


            int it = 0;
            // заполняем черным и белым цветом картинку
            for (int i = 0; i < image.height; i++){
                for (int j = 0; j < image.width; j++){
                    imgbytes[it] = 255 * !img[i][j] ;
                    imgbytes[it+1] = 255 * !img[i][j];
                    imgbytes[it+2] = 255 * !img[i][j];
                    it+=3;}

                    it+=padding; // прибавляем выравнивание
            }

            fwrite(imgbytes, 1, image.size , life); // записываем в новый файл изображение
            fclose(life); // закрываем созданный файл
        }

        game_of_life(img, image.height, image.width); // включаем игру
        iter++; // увеличиваем жизнь
    
    }

    return 0;
}

// ./game --input example.bmp --output /Users/ /Desktop/example --max_iter 100 --dump_freq 10
