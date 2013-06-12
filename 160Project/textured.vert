#version 400

in vec3 vPosition;
in vec3 vNormal;
in vec2 vertexUV;

uniform mat4 Projection;
uniform mat4 ModelView;
uniform mat4 mTransformation;

uniform vec4 AmbientProduct;
uniform vec4 DiffuseReflect;
uniform vec4 LightSpecular;
uniform vec4 MaterialSpecular;
uniform vec4 LightPosition;
uniform vec4 colorID;
uniform vec4 LightDiffuse;
uniform float Shininess;

//Output that will be interpolated per fragment.
out vec2 UV;
out vec4 color;

subroutine vec4 colorType(mat4 ModelView_);
subroutine uniform colorType colorModel;

subroutine(colorType)
vec4 normalMode(mat4 ModelView_){
	vec4 DiffuseProduct = LightDiffuse * DiffuseReflect;
	vec3 eyePos = (ModelView_ * vec4(vPosition, 1.0)).xyz;
	//Take upper three from modelview
    vec3 eyeNorm = normalize(inverse(transpose(mat3(ModelView_))) * vNormal);
	vec3 L;
	if(LightPosition.w == 0.0)
		L = normalize(LightPosition.xyz);
	else
		L = normalize((LightPosition).xyz - eyePos); //light source direction in view coords (s)

	//ambient light is the same for any vertex
	vec4 ambient = AmbientProduct;
	//diffuse reflection coefficient (sDotN)
	float sDotN = max(dot(L, eyeNorm), 0.0 );
	
	//calculate diffuse intensity
	vec4 diffuse = DiffuseProduct * sDotN;

	vec3 E = normalize(-eyePos.xyz);
	//reflection vertex
	vec3 R = reflect(-L, eyeNorm);

	//default when no light is reaching surface
	vec4 specular = vec4(0.0, 0.0, 0.0, 1.0);
	if(sDotN > 0.0){
		//specular reflection coefficient
		float Ks = pow(max(dot(R,E),0.0), Shininess);
		specular =  LightSpecular * MaterialSpecular * Ks;
	}

	return vec4((specular + diffuse + ambient).xyz, 1.0 );
}

subroutine(colorType)
vec4 colorKeyMode(mat4 ModelView_){
	return colorID;
}
void main()
{
	mat4 ModelView_ = ModelView * mTransformation;
	gl_Position = Projection * ModelView_ * vec4(vPosition, 1.0);
	color = colorModel(ModelView_);
	UV = vertexUV;
}
