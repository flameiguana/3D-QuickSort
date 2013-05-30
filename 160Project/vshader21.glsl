#version 150 


in  vec4 vPosition;
in  vec3 normal;
out vec4 color;

uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform vec4 material;
uniform mat4 camera;
uniform mat4 model;
uniform mat4 projection;
uniform vec3 light;
uniform vec4 eye;
uniform float Shininess;
uniform vec3 coloringOn;
void main()
{

	mat4 model_view = camera*model;
    // Transform vertex  position into eye coordinates
    vec3 pos = (model_view * vPosition).xyz;
	
    vec3 L = normalize( light );
    vec3 E = normalize( (eye.xyz) );
    vec3 H = normalize( L + E );

    // Transform vertex normal into eye coordinates
    vec3 N = normalize( inverse(transpose(model))*vec4(normal, 0.0) ).xyz;

    // Compute terms in the illumination equation
    vec4 ambient = AmbientProduct*material;

    float Kd = max( dot(L, N), 0.0 );
    vec4  diffuse = Kd*DiffuseProduct*material;

    float Ks = pow( max(dot(N, H), 0.0), 50 );
    vec4  specular = Ks * SpecularProduct;
    
	color = ambient + diffuse + specular*material;
    if( dot(L, N) < 0.0 ) {
		specular = vec4(0.0, 0.0, 0.0, 1.0);
		diffuse = vec4(0.0, 0.0, 0.0, 1.0);
		color = vec4(0.0, 0.0, 0.0, 1.0);
    } 

    gl_Position = projection * model_view * vPosition;

    color.a = 1.0;
}