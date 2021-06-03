#include <string.h>
#include "stdio.h"
#include <math.h>

// Sobel Filter ACC
static char* const SOBELFILTER_RS_START_ADDR = reinterpret_cast<char* const>(0x74000000);
static char* const SOBEL_FILTER_RS_W_WIDTH   = reinterpret_cast<char* const>(0x74000004);
static char* const SOBEL_FILTER_RS_W_HEIGHT  = reinterpret_cast<char* const>(0x74000008);
static char* const SOBEL_FILTER_RS_W_DATA    = reinterpret_cast<char* const>(0x7400000C);
static char* const SOBELFILTER_RS_READ_ADDR  = reinterpret_cast<char* const>(0x74800000);

// DMA 
static volatile uint32_t * const DMA_SRC_ADDR  = (uint32_t * const)0x70000000;
static volatile uint32_t * const DMA_DST_ADDR  = (uint32_t * const)0x70000004;
static volatile uint32_t * const DMA_LEN_ADDR  = (uint32_t * const)0x70000008;
static volatile uint32_t * const DMA_OP_ADDR   = (uint32_t * const)0x7000000C;
static volatile uint32_t * const DMA_STAT_ADDR = (uint32_t * const)0x70000010;
static const uint32_t DMA_OP_MEMCPY = 1;
static const uint32_t DMA_TRANS_NUM = 64;

bool _is_using_dma = true;

unsigned int ReadfromByteArray(unsigned char* array, unsigned int offset) {
	unsigned int output = (array[offset] << 0) | (array[offset + 1] << 8) | (array[offset + 2] << 16) | (array[offset + 3] << 24);
	return output;
}

void write_data_to_ACC(char* ADDR, unsigned char* buffer, int len){
  if(_is_using_dma){  
    // Using DMA 
    *DMA_SRC_ADDR = (uint32_t)(buffer);
    *DMA_DST_ADDR = (uint32_t)(ADDR);
    *DMA_LEN_ADDR = len;
    *DMA_OP_ADDR  = DMA_OP_MEMCPY;
  }else{
    // Directly Send
    memcpy(ADDR, buffer, sizeof(unsigned char)*len);
  }
}
void read_data_from_ACC(char* ADDR, unsigned char* buffer, int len){
  if(_is_using_dma){
    // Using DMA 
    *DMA_SRC_ADDR = (uint32_t)(ADDR);
    *DMA_DST_ADDR = (uint32_t)(buffer);
    *DMA_LEN_ADDR = len;
    *DMA_OP_ADDR  = DMA_OP_MEMCPY;
  }else{
    // Directly Read
    memcpy(buffer, ADDR, sizeof(unsigned char)*len);
  }
}

void partition_base_input_data(unsigned char* buffer, unsigned int data){
    for(int i = 0; i < 4; i++)
      buffer[i] = (i == 0) ? data % 256 : data / pow(256, i);;
}

int main() {
  #include "lena_std_short.h" //included here to avoid compiler issue of not initializing global arrays
  #include "test.h" //included here to avoid compiler issue of not initializing global arrays
	unsigned char* source_array= test_bmp;

  unsigned int input_rgb_raw_data_offset = ReadfromByteArray(source_array, 10);
	unsigned int width = ReadfromByteArray(source_array, 18);
	unsigned int height = ReadfromByteArray(source_array, 22);
	unsigned int bytes_per_pixel = ReadfromByteArray(source_array, 28) / 8;
	unsigned char* source_bitmap = &source_array[input_rgb_raw_data_offset];
  printf("======================================\n");
  printf("\t  Reading from array\n");
  printf("======================================\n");
	printf(" input_rgb_raw_data_offset\t= %d\n", input_rgb_raw_data_offset);
	printf(" width\t\t\t\t= %d\n", width);
	printf(" height\t\t\t\t= %d\n", height);
	printf(" bytes_per_pixel\t\t= %d\n",bytes_per_pixel);
  printf("======================================\n");

  unsigned char  width_buffer[4] = {0};
  unsigned char  height_buffer[4] = {0};
  unsigned char  buffer[DMA_TRANS_NUM] = {0};
  unsigned char  result_buffer[width] = {0};

  partition_base_input_data(width_buffer, width);
  partition_base_input_data(height_buffer, height);

  write_data_to_ACC(SOBEL_FILTER_RS_W_WIDTH , width_buffer, 4);
  write_data_to_ACC(SOBEL_FILTER_RS_W_HEIGHT, height_buffer, 4);
  int c = 0;
  int trans_count = 0;
  // for(int i = 0; i < 3; i++){
  for(int i = 0; i < height + 1 ; i++){
    for(int j = 0; j < width; j++){
      // RGB
      if(i < height)
        for(int k = 0; k < 3; k++){
          buffer[(j * 3 + k) % DMA_TRANS_NUM] = *(source_bitmap + bytes_per_pixel * (i) * width + (j) + k); 
          // buffer[(j * 3 + k) % DMA_TRANS_NUM] = j; 
          trans_count ++;
          if(trans_count == DMA_TRANS_NUM){
            trans_count = 0;
            write_data_to_ACC(SOBEL_FILTER_RS_W_DATA, buffer, 64);
          }
        }
      else
        for(int k = 0; k < 3; k++){
          buffer[(j * 3 + k) % DMA_TRANS_NUM] = 0; 
          trans_count ++;
          if(trans_count == DMA_TRANS_NUM){
            trans_count = 0;
            write_data_to_ACC(SOBEL_FILTER_RS_W_DATA, buffer, 64);
          }
        }
    }
    
    for(int i = 0; i < 4; i++){
      read_data_from_ACC(SOBELFILTER_RS_READ_ADDR, result_buffer, 64); 
      for(int j = 0; j < DMA_TRANS_NUM; j++ )
          printf("%d,",(unsigned int) result_buffer[j]);              
    }
    printf("\n");
    
  }
}
