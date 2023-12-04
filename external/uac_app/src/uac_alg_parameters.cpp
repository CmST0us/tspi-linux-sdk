/*
 * Copyright 2020 Rockchip Electronics Co. LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "uac_alg_parameters.h"
#include <stdio.h>

int aec_write_paramters() {
    /*
     * default nlp parameters
     */
    const short nlp[8][2] = {
        /* BandPassThd      SuperEstFactor */
        {      10,                 6      },     /* Hz: 0    - 300  */
        {      10,                 6      },     /* Hz: 300  - 575  */
        {      10,                 6      },     /* Hz: 575  - 950  */
        {      5,                  6      },     /* Hz: 950  - 1425 */
        {      5,                  6      },     /* Hz: 1425 - 2150 */
        {      5,                  6      },     /* Hz: 2150 - 3350 */
        {      0,                  6      },     /* Hz: 3350 - 5450 */
        {      0,                  6      },     /* Hz: 5450 - 8000 */
    };

    /*
     * default nlp plus parameters
     */
    const short nlpPlus[8][7] = {
        /* Enable   FarThd   ErrEchoThd_L     ErrEchoThd_H     ErrRatioThd_L    ErrRatioThd_H    NlineDegree */
        {   0,        0,         0,              10,             9830,             16383,            1        },   /* Hz: 0    - 300  */
        {   0,        0,         0,              10,             9830,             16383,            1        },   /* Hz: 300  - 575  */
        {   0,        0,         0,              10,             9830,             16383,            1        },   /* Hz: 575  - 950  */
        {   0,        0,         0,              10,             9830,             16383,            1        },   /* Hz: 950  - 1425 */
        {   0,        0,         5,              20,             9830,             16383,            1        },   /* Hz: 1425 - 2150 */
        {   0,        0,         5,              30,             9830,             16383,            1        },   /* Hz: 2150 - 3350 */
        {   0,        0,         10,             50,             9830,             16383,            1        },   /* Hz: 3350 - 5450 */
        {   0,        0,         50,             300,            9830,             16383,            1        }    /* Hz: 5450 - 8000 */
    };
    printf("aec_write_paramters\n");
    // write nlp parameters
    FILE* file = fopen("/userdata/nlp.bin", "wb");
    if (file != NULL) {
        fwrite(nlp, sizeof(short), 8*2, file);
        fflush(file);
        fclose(file);
    }

    // write nlp plus parameters
    file = fopen("/userdata/nlp_plus.bin", "wb");
    if (file != NULL) {
        fwrite(nlpPlus, sizeof(short), 8*7, file);
        fflush(file);
        fclose(file);
    }

    return 0;
}
