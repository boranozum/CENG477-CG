// glScalef(grid[i][j].size, grid[i][j].size, grid[i][j].size); 

            GLint color = glGetUniformLocation(gProgram, "colorIndex");
            glUniform1i(color, grid[i][j].color_index);

            //findMatches();

            if(grid[i][j].will_explode && !grid[i][j].exploded){
                if(grid[i][j].size <= grid[i][j].explode_size){
                    grid[i][j].size += grid[i][j].size*0.01;
                    glScalef(grid[i][j].size, grid[i][j].size, grid[i][j].size); 
                }
                else{
                    // grid[i][j].size = 25.0/48.0;
                    grid[i][j].will_explode = false;
                    grid[i][j].exploded = true;
                }
            }
            else if (!grid[i][j].exploded){
                glScalef(grid[i][j].size, grid[i][j].size, grid[i][j].size); 
            }
            else{
                if(j != grid_h-1){
                    for (int k = j+1; k < grid_h; k++)
                    {
                        if(grid[i][k].y != grid[i][k-1].y){
                            grid[i][k].y = grid[i][k].y - 0.05;
                        }    

                        if(grid[i][k].y >= grid[i][k-1].y - 0.05 && grid[i][k].y <= grid[i][k-1].y + 0.05){
                            grid[i][k].y = grid[i][k-1].y;
                            grid[i][j].exploded = false;
                            for(int l = j; l < grid_h; l++){
                                grid[i][l] = grid[i][l+1];
                            }
                            grid[i][grid_h-1] = Cell(-10+(20.0/(grid_w+1))+20*i/(grid_w+1), (-10+(20.0/(grid_h+1))+20*grid_h/(grid_h+1)), rand() % 5);
                            
                        }
                    }
                }
                continue;
                // else{
                    
                // }
            }

            drawModel();