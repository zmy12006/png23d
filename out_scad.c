/*
 * Copyright 2011 Vincent Sanders <vince@kyllikki.org>
 *
 * Licenced under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 *
 * This file is part of png23d. 
 * 
 * Routines to output in SCAD format
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include "option.h"
#include "bitmap.h"
#include "out_scad.h"

static void 
output_scad_cube(FILE *outf, int x,int y, int z, int width, int height, int depth)
{
    fprintf(outf,
            "        translate([%d, %d, %d]) cube([%d.01, %d.01, %d.01]);\n",
            x, y, z,
            width, height, depth);
}

/* generate scad output as rows of cubes */
bool output_flat_scad_cubes(bitmap *bm, int fd, options *options)
{
    int xoff; /* x offset so 3d model is centered */
    int yoff; /* y offset so 3d model is centered */
    unsigned int row_loop;
    unsigned int col_loop;
    unsigned int col_start; /* start col of run */
    unsigned int xmin = bm->width;
    unsigned int xmax = 0;
    unsigned int ymin = bm->height;
    unsigned int ymax = 0;
    FILE *outf;

    outf = fdopen(dup(fd), "w");

    xoff = (bm->width / 2);
    yoff = (bm->height / 2);

    fprintf(outf, "// Generated by png2scad\n\n");

    fprintf(outf, "target_width = %f;\n", options->width);
    fprintf(outf, "target_depth = %f;\n\n", options->depth);

    fprintf(outf, "module image(sx,sy,sz) {\n scale([sx, sy, sz]) union() {\n");

    for (row_loop = 0; row_loop < bm->height; row_loop++) {
        col_start = bm->width;
        for (col_loop = 0; col_loop < bm->width; col_loop++) {
            
            if (bm->data[(row_loop * bm->width) + col_loop] < options->transparent) {
                /* this cell is "opaque" */
                if (col_start > col_loop) {
                    /* mark start of run */
                    col_start = col_loop;
                }
                if (col_loop < xmin)
                    xmin = col_loop;
                if (col_loop > xmax)
                    xmax = col_loop;
                if (row_loop < ymin)
                    ymin = row_loop;
                if (row_loop > ymax)
                    ymax = row_loop;

            } else {
                /* cell is transparent */
                if (col_start < col_loop) {
                    /* output previous run */
                    output_scad_cube(outf,
                                     col_start - xoff, yoff - row_loop , 0,
                                     col_loop - col_start, 1, 1);
                    col_start = bm->width; /* ready for next run */
                }
            }
        }
        /* need to close any active run at edge of image */
        if (col_start < col_loop) {
            /* output active run */
            output_scad_cube(outf,
                             col_start - xoff, row_loop - yoff, 0,
                             col_loop - col_start, 1, 1);
        }

    }

    fprintf(outf, "    }\n}\n\n");
    fprintf(outf, "image_width = %d;\n", xmax - xmin);
    fprintf(outf, "image_height = %d;\n\n", ymax - ymin);

    fprintf(outf, "image(target_width / image_width, target_width / image_width, target_depth);\n");

    return true;
}
