# Parallel Image Processing
A coursework for my uni. Image processing using parallel and sequential programming with comparison of the execution time.

[Source file](RGB_ProcessingWin/main.cpp)

Images render_bottom_1.png, render_bottom_2.png, render_top_1.png, render_top_2.png from Images/ folder were processed in 3 stages.

Stage 1 - Image comparison

Stage 2 - Blur & PostProcessing

Stage 3 - Image Mask

Results of each stage:
- [S1 bottom](Images/stage1_bottom.png 	)
- [S1 top](Images/stage1_top.png)
- [S1 combined](Images/stage1_combined.png )
- [S2 blurred](Images/stage2_blurred.png)
- [S2 threshold](Images/stage2_threshold.png)
- [S3](Images/stage3.png)

Gaussian Blur - A function to create a blur kernel with size modified by a user was implemented.

Using FreeImagePlus library.
Stage 1 uses thread library.
Stage 2 and 3 use Thread building blocks library.
