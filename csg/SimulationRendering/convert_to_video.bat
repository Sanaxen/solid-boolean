ffmpeg -r 2 -y -i png\%%d.png -vcodec libx264 -pix_fmt yuv420p  sim_.mp4
ffmpeg -i sim_.mp4 -filter_complex "[0:v] fps=10,scale=640:-1,split [a][b];[a] palettegen [p];[b][p] paletteuse" sim_.gif
