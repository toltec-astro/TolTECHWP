
void *QuadBufferToDisk(void *input){
    // get file path
    char *savefilepath = ini_get(((struct p_args*)input)->config, "savefilepath", "path");
    
    // generate file name
    char fname[200];
    time_t save_date;
    time(&save_date);
    strftime(fname, 100, "%y-%m-%d_%H-%M_quad_data.txt", localtime(&save_date));

    // construct final location
    char full_path[200];
    sprintf(full_path, "%s%s", savefilepath, fname);

    // set location 
    FILE *outfile;
    outfile = fopen(full_path, "wt");

    // text out buffer
    char txt_out_buf[500];
    int cycle = 0;

    // print header
    fprintf(outfile, "buffer_id, count, cardcounter, computertime\n");

    // TODO: REPEATING LOGIC (you can do better)
    while (1) {
        // if the bottom is ready  printf("Quad Bottom: %i, Quad Top: %i \n", quad_bot_flag, quad_top_flag);
        
        // read the bottom
        if ((quad_in_ptr > QUAD_BUFFER_LENGTH/2) && (cycle == 0)){
            
            for (int i = 0; i < QUAD_BUFFER_LENGTH/2; i++) {

                sprintf(txt_out_buf, "%d, %d, %u, %u \n", 
                      i, quad_counter[i], quad_card_time[i], quad_cpu_time[i]
                );
                fprintf(outfile, "%s", txt_out_buf);
                continue;
            }
            cycle = 1;
        }

        // read the top
        if ((quad_in_ptr < QUAD_BUFFER_LENGTH/2) && (cycle == 1)){
            for (int i = QUAD_BUFFER_LENGTH/2; i < QUAD_BUFFER_LENGTH; i++) {

                sprintf(txt_out_buf, "%d, %d, %u, %u \n", 
                      i, quad_counter[i], quad_card_time[i], quad_cpu_time[i]
                );
                fprintf(outfile, "%s", txt_out_buf);
                continue;
            }
            cycle = 0;
        }
    } 
    return 0;
}


void *SensorBufferToDisk(void *input){

    // get file path
    char *savefilepath = ini_get(((struct p_args*)input)->config, "savefilepath", "path");
    
    // generate file name
    char fname[200];
    time_t save_date;
    time(&save_date);
    strftime(fname, 100, "%y-%m-%d_%H-%M_sensor_data.txt", localtime(&save_date));

    // construct final location
    char full_path[200];
    sprintf(full_path, "%s%s", savefilepath, fname);

    // set location 
    FILE *outfile;
    outfile = fopen(full_path, "wt");

    // text out buffer
    char txt_out_buf[500];
    int cycle = 0;

    // print header
    fprintf(outfile, "buffer_id, sensor_id, sensor_voltage_volts, computertime\n");

    // TODO: REPEATING LOGIC (you can do better)
    while (1) {
        
        // read the bottom
        if ((sensor_in_ptr > BUFFER_LENGTH/2) && (cycle == 0)){
            
            for (int i = 0; i < BUFFER_LENGTH/2; i++) {

                sprintf(txt_out_buf, "%i, %i, %f, %d \n", 
                      i, sensor_id[i], sensor_voltage[i], sensor_cpu_time[i]
                );
                fprintf(outfile, "%s", txt_out_buf);
                continue;            
            }
            cycle = 1;
        }

        // read the top
        if ((sensor_in_ptr < BUFFER_LENGTH/2) && (cycle == 1)){
            for (int i = BUFFER_LENGTH/2; i < BUFFER_LENGTH; i++) {
                sprintf(txt_out_buf, "%i, %i, %f, %d  \n", 
                      i, sensor_id[i], sensor_voltage[i], sensor_cpu_time[i]
                );
                fprintf(outfile, "%s", txt_out_buf);
                continue;
            }
            cycle = 0;
        }
        sleep(3);
    } 
    return 0;
}

void *PPSBufferToDisk(void *input){

    // get file path
    char *savefilepath = ini_get(((struct p_args*)input)->config, "savefilepath", "path");
    
    // generate file name
    char fname[200];
    time_t save_date;
    time(&save_date);
    strftime(fname, 100, "%y-%m-%d_%H-%M_pps_data.txt", localtime(&save_date));

    // construct final location
    char full_path[200];
    sprintf(full_path, "%s%s", savefilepath, fname);

    // set location 
    FILE *outfile;
    outfile = fopen(full_path, "wt");

    // text out buffer
    char txt_out_buf[500];
    int cycle = 0;

    // print header
    fprintf(outfile, "buffer_id, count, cardcounter, computertime\n");

    // TODO: REPEATING LOGIC (you can do better)
    while (1) {
        
        // read the bottom
        if ((pps_in_ptr > BUFFER_LENGTH/2) && (cycle == 0)){
            
            for (int i = 0; i < BUFFER_LENGTH/2; i++) {
                
                sprintf(txt_out_buf, "%d, %d, %u, %u \n", 
                      i, pps_id[i], pps_card_time[i], pps_cpu_time[i]
                );

                fprintf(outfile, "%s", txt_out_buf);
                //printf("PPS: %d, %d, %d, %d \n", 
                //      sizeof(i), sizeof(pps_id[i]), sizeof(pps_card_time[i]), sizeof(pps_cpu_time[i])
                //);
                continue;            
            }
            cycle = 1;
        }

        // read the top
        if ((pps_in_ptr < BUFFER_LENGTH/2) && (cycle == 1)){
            for (int i = BUFFER_LENGTH/2; i < BUFFER_LENGTH; i++) {

                sprintf(txt_out_buf, "%d, %d, %u, %u \n", 
                      i, pps_id[i], pps_card_time[i], pps_cpu_time[i]
                );
                fprintf(outfile, "%s", txt_out_buf);
                continue;
            }
            cycle = 0;
        }

        sleep(3);
    } 
    return 0;
}

    //pthread_join(quad_write_thread, NULL);
    //pthread_join(pps_write_thread, NULL);
    //pthread_join(sensor_write_thread, NULL);
    //pthread_join(publish_thread, NULL);