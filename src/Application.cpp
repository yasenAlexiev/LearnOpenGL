#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>


#include "Renderer.h"

#include "IndexBuffer.h"
#include "VertexBuffer.h"

#include "VertexArray.h"

struct ShaderProgramSource
{
	std::string VertexSource;
	std::string FragmentSource;
};

static ShaderProgramSource ParseShader( const std::string& filePath )
{
	std::fstream stream( filePath );

	enum class ShaderType
	{
		NONE = -1, VERTEX = 0, FRAGMENT = 1
	};

	std::string line;
	std::stringstream ss[ 2 ];
	ShaderType type = ShaderType::NONE;

	while ( getline( stream, line ) )
	{
		if ( line.find( "#shader" ) != std::string::npos )
		{
			if ( line.find( "vertex" ) != std::string::npos )
			{
				type = ShaderType::VERTEX;
			}
			else if ( line.find( "fragment" ) != std::string::npos )
			{
				type = ShaderType::FRAGMENT;
			}
		}
		else
		{
			ss[ (int) type ] << line << '\n';
		}
	}

	return { ss[ 0 ].str(), ss[ 1 ].str() };
}

static int CompileShader( unsigned int type, const std::string& source )
{
	unsigned int id = glCreateShader( type );
	const char* src = source.c_str();

	GLCall( glShaderSource( id, 1, &src, nullptr ) );
	GLCall( glCompileShader( id ) );

	int result;
	GLCall( glGetShaderiv( id, GL_COMPILE_STATUS, &result ) );
	if ( result == GL_FALSE )
	{
		int length;
		GLCall( glGetShaderiv( id, GL_INFO_LOG_LENGTH, &length ) );
		char* message = (char*) alloca( length * sizeof( char ) );
		GLCall( glGetShaderInfoLog( id, length, &length, message ) );
		std::cout << "Failed to compile " << ( type == GL_VERTEX_SHADER ? "vextex" : "fragment" ) << std::endl;
		std::cout << message << std::endl;
		GLCall( glDeleteShader( id ) );
	}

	return id;
}

static unsigned int CreateShader( const std::string& vertexShader, const std::string& fragmentShader )
{
	unsigned int program = glCreateProgram();
	unsigned int vs = CompileShader( GL_VERTEX_SHADER, vertexShader );
	unsigned int fs = CompileShader( GL_FRAGMENT_SHADER, fragmentShader );

	GLCall( glAttachShader( program, vs ) );
	GLCall( glAttachShader( program, fs ) );
	GLCall( glLinkProgram( program ) );
	GLCall( glValidateProgram( program ) );

	GLCall( glDeleteShader( vs ) );
	GLCall( glDeleteShader( fs ) );

	return program;
}

int main( void )
{
	GLFWwindow* window;

	/* Initialize the library */
	if ( !glfwInit() )
		return -1;

	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );


	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow( 640, 480, "Hello World", NULL, NULL );
	if ( !window )
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent( window );

	glfwSwapInterval( 1 );

	if ( glewInit() != GLEW_OK )
	{
		std::cout << "Error" << std::endl;
	}

	std::cout << glGetString( GL_VERSION ) << std::endl;

	float positions[] = {
		-0.5f, -0.5f,
		 0.5f, -0.5f,
		 0.5f,  0.5f,
		-0.5f,  0.5f,
	};

	unsigned int indices[] = {
		0, 1, 2,
		2, 3, 0
	};

	unsigned int vao;
	GLCall( glGenVertexArrays( 1, &vao ) );
	GLCall( glBindVertexArray( vao ) );

	VertexArray va;
	VertexBuffer vb( positions, 4 * 2 * sizeof( float ) );

	VertexBufferLayout layout;
	layout.Push<float>( 2 );
	va.AddBuffer( vb, layout );

	IndexBuffer ib( indices, 6 );

	ShaderProgramSource source = ParseShader( "res/shaders/Basic.shader" );
	unsigned int shader = CreateShader( source.VertexSource, source.FragmentSource );
	GLCall( glUseProgram( shader ) );

	GLCall( int location = glGetUniformLocation( shader, "u_Color" ) );
	ASSERT( location != -1 );
	GLCall( glUniform4f( location, 0.7f, 0.3f, 0.8f, 1.0 ) );

	GLCall( glBindVertexArray( 0 ) );
	GLCall( glUseProgram( 0 ) );
	GLCall( glBindBuffer( GL_ARRAY_BUFFER, 0 ) );
	GLCall( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 ) );


	float r = 0.0f;
	float increment = 0.05f;

	/* Loop until the user closes the window */
	while ( !glfwWindowShouldClose( window ) )
	{
		/* Render here */
		GLCall( glClear( GL_COLOR_BUFFER_BIT ) );

		GLCall( glUseProgram( shader ) );
		GLCall( glUniform4f( location, r, 0.3f, 0.8f, 1.0 ) );

		va.Bind();
		ib.Bind();

		GLCall( glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr ) );

		if ( r > 1.0f )
		{
			increment = -0.05f;
		}
		else if ( r < 0.0f )
		{
			increment = 0.05f;
		}

		r += increment;

		/* Swap front and back buffers */
		glfwSwapBuffers( window );

		/* Poll for and process events */
		glfwPollEvents();
	}

	glDeleteProgram( shader );

	glfwTerminate();
	return 0;
}