# HW6 Report 蕭詠倫 109061634

### 結果

![image](https://github.com/lplp9312/ee6470_hw6/blob/master/gau_sim.jpg)

### Testbench code : 傳送和接收 RGB
    
      unsigned char  buffer[4] = {0};
      for(int i = 0; i < width; i++){
        for(int j = 0; j < length; j++){
          for(int v = -1; v <= 1; v ++){
            for(int u = -1; u <= 1; u++){
              if((v + i) >= 0  &&  (v + i ) < width && (u + j) >= 0 && (u + j) < length ){
                buffer[0] = *(source_bitmap + bytes_per_pixel * ((j + u) * width + (i + v)) + 2);
                buffer[1] = *(source_bitmap + bytes_per_pixel * ((j + u) * width + (i + v)) + 1);
                buffer[2] = *(source_bitmap + bytes_per_pixel * ((j + u) * width + (i + v)) + 0);
                buffer[3] = 0;
              }else{
                buffer[0] = 0;
                buffer[1] = 0;
                buffer[2] = 0;
                buffer[3] = 0;
              }
              write_data_to_ACC(GAU_START_ADDR, buffer, 4);

            }
          }
          read_data_from_ACC(GAU_READ_ADDR, buffer, 4);
        }
      }

### Filter code : 進行卷積

        void do_filter(){
        { wait(CLOCK_PERIOD, SC_NS); }
        while (true) {
          int r_val = 0; 
          int g_val = 0; 
          int b_val = 0;
          for (unsigned int v = 0; v < MASK_Y; ++v) {
            for (unsigned int u = 0; u < MASK_X; ++u) {

              unsigned char r = i_r.read();
              unsigned char g = i_g.read();
              unsigned char b = i_b.read();
              wait(CLOCK_PERIOD, SC_NS);
              r_val += r * mask1[u][v];
              g_val += g * mask1[u][v];
              b_val += b * mask1[u][v];        
              wait(CLOCK_PERIOD, SC_NS);
            }
          }

          o_result_r.write(r_val / 16);    
          o_result_g.write(g_val / 16);
          o_result_b.write(b_val / 16);
        }
      }


### 心得

####本次作業在修改程式碼上還蠻簡單的，但是要詳細了解 RISC-V VP platform 不同 moudule 上的運作情形和作用需要花上不少時間。


