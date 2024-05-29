
#include "vtk_base64_output_stream.h"
#include "vtk_base64.h"


//------------------------------------------------------------------------------
vtkBase64OutputStream::vtkBase64OutputStream(std::ofstream& _stream): stream(_stream),BufferLength(0)
{

}

//------------------------------------------------------------------------------
vtkBase64OutputStream::~vtkBase64OutputStream() = default;


//------------------------------------------------------------------------------
inline int vtkBase64OutputStream::EncodeTriplet(
  unsigned char c0, unsigned char c1, unsigned char c2)
{
  // Encodes 3 bytes into 4 bytes and writes them to the output stream.
  unsigned char out[4];
  vtkBase64::EncodeTriplet(c0, c1, c2, &out[0], &out[1], &out[2], &out[3]);
  return (this->stream.write(reinterpret_cast<char*>(out), 4) ? 1 : 0);
}

//------------------------------------------------------------------------------
inline int vtkBase64OutputStream::EncodeEnding(unsigned char c0, unsigned char c1)
{
  // Encodes a 2-byte ending into 3 bytes and 1 pad byte and writes.
  unsigned char out[4];
  vtkBase64::EncodePair(c0, c1, &out[0], &out[1], &out[2], &out[3]);
  return (this->stream.write(reinterpret_cast<char*>(out), 4) ? 1 : 0);
}

//------------------------------------------------------------------------------
inline int vtkBase64OutputStream::EncodeEnding(unsigned char c0)
{
  // Encodes a 1-byte ending into 2 bytes and 2 pad bytes and writes.
  unsigned char out[4];
  vtkBase64::EncodeSingle(c0, &out[0], &out[1], &out[2], &out[3]);
  return (this->stream.write(reinterpret_cast<char*>(out), 4) ? 1 : 0);
}

//------------------------------------------------------------------------------
int vtkBase64OutputStream::StartWriting()
{
  this->BufferLength = 0;
  return 1;
}

//------------------------------------------------------------------------------
int vtkBase64OutputStream::EndWriting()
{
  if (this->BufferLength == 1)
  {
    if (!this->EncodeEnding(this->Buffer[0]))
    {
      return 0;
    }
    this->BufferLength = 0;
  }
  else if (this->BufferLength == 2)
  {
    if (!this->EncodeEnding(this->Buffer[0], this->Buffer[1]))
    {
      return 0;
    }
    this->BufferLength = 0;
  }
  return 1;
}

//------------------------------------------------------------------------------
int vtkBase64OutputStream::Write(void const* data, size_t length)
{
  size_t totalLength      = this->BufferLength + length;
  unsigned char const* in = static_cast<unsigned char const*>(data);
  unsigned char const* end = in + length;

  if (totalLength >= 3)
  {
    if (this->BufferLength == 1)
    {
      if (!this->EncodeTriplet(this->Buffer[0], in[0], in[1]))
      {
        return 0;
      }
      in += 2;
      this->BufferLength = 0;
    }
    else if (this->BufferLength == 2)
    {
      if (!this->EncodeTriplet(this->Buffer[0], this->Buffer[1], in[0]))
      {
        return 0;
      }
      in += 1;
      this->BufferLength = 0;
    }
  }

  while ((end - in) >= 3)
  {
    if (!this->EncodeTriplet(in[0], in[1], in[2]))
    {
      return 0;
    }
    in += 3;
  }

  while (in != end)
  {
    this->Buffer[this->BufferLength++] = *in++;
  }
  return 1;
}
