#version 400

in vec3 vPosition;
in vec3 vNormal;
uniform mat4 Projection;
uniform mat4 ModelView;
uniform mat4 mTransformation;

uniform vec4 AmbientProduct;
vec4 DiffuseProduct; //Diffuse reflectivity * intensity
uniform vec4 SpecularProduct;
uniform vec4 LightPosition;
uniform vec4 colorID;

uniform vec4 LightDiffuse;
uniform float Shininess;
out vec4 color;

const vec4 grass = vec4(.8, .8, .8, 1.0);

subroutine vec4 shadeModelType(vec3 eyePos, vec3 eyeNorm, vec3 L);
subroutine uniform shadeModelType shadeModel;


subroutine(shadeModelType)
//Ambient and diffuse
vec4 both(vec3 eyePos, vec3 eyeNorm, vec3 L)
{
	//ambient light is the same for any vertex
	vec4 ambient = AmbientProduct;
	//diffuse reflection coefficient (sDotN)
    float sDotN = max(dot(L, eyeNorm), 0.0 );

	//calculate diffuse intensity
    vec4 diffuse = DiffuseProduct*sDotN;
	return ambient + diffuse;
}

subroutine(shadeModelType)
vec4 diffuseOnly(vec3 eyePos, vec3 eyeNorm, vec3 L)
{
    return DiffuseProduct * max(dot(L, eyeNorm), 0.0 );
}

subroutine(shadeModelType)
vec4 ambientOnly(vec3 eyePos, vec3 eyeNorm, vec3 L)
{
	return AmbientProduct;
}

subroutine(shadeModelType)
vec4 colorKey(vec3 eyePos, vec3 eyeNorm, vec3 L)
{
	return colorID;	
}
//Switches for enabling or disabling specular calculation.
subroutine vec4 specularType(vec3 eyePos, vec3 L, vec3 eyeNorm);
subroutine uniform specularType specularModel;

subroutine(specularType)
vec4 specularOn(vec3 eyePos, vec3 eyeNorm, vec3 L){

	//direction of reflection towards viewer (v)
	vec3 E = normalize(-eyePos.xyz);
	//reflection vertex
	vec3 R = reflect(-L, eyeNorm);

    float sDotN = max(dot(L,eyeNorm), 0.0 );
	//default when no light is reaching surface
	vec4 specular = vec4(0.0, 0.0, 0.0, 1.0);
	if(sDotN > 0.0){
		//specular reflection coefficient
		float Ks = pow(max(dot(R,E),0.0), Shininess);
		specular =  SpecularProduct * Ks;
	}
	return specular;
}

subroutine(specularType)
vec4 specularOff(vec3 eyePos, vec3 eyeNorm, vec3 L){
    return vec4(0.0, 0.0, 0.0, 1.0);
}

void main()
{
	mat4 ModelView_ = ModelView * mTransformation;
	DiffuseProduct = LightDiffuse * grass;
	//the eye coordinates of position

	vec3 eyePos = (ModelView_ * vec4(vPosition, 1.0)).xyz;
	//Take upper three from modelview
    vec3 eyeNorm = normalize(inverse(transpose(mat3(ModelView_))) * vNormal);
	vec3 L;
	if(LightPosition.w == 0.0)
		L = normalize(LightPosition.xyz);
	else
		L = normalize((LightPosition).xyz - eyePos); //light source direction in view coords (s)

    gl_Position = Projection * ModelView_ * vec4(vPosition, 1.0);
    color = vec4((shadeModel(eyePos, eyeNorm, L).xyz + specularModel(eyePos, eyeNorm, L).xyz), 1.0 );
}
