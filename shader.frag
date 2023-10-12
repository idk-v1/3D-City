uniform sampler2D texture;

void main()
{
    vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);

	if (pixel.r == 0.0 && pixel.g == 0.0 && pixel.b == 0.0)
		gl_FragColor = vec4(0, 0, 0, 0);
	else
    	gl_FragColor = pixel;
}