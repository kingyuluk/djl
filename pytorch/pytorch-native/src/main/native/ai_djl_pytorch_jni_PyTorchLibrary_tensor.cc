/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You may not use this file except in compliance
 * with the License. A copy of the License is located at
 *
 * http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
 * OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions
 * and limitations under the License.
 */
#include "ai_djl_pytorch_jni_PyTorchLibrary.h"
#include "djl_pytorch_jni_exception.h"
#include "djl_pytorch_jni_utils.h"

// The file is the implementation for PyTorch tensor core functionality operation

JNIEXPORT jlongArray JNICALL Java_ai_djl_pytorch_jni_PyTorchLibrary_torchSizes(
    JNIEnv* env, jobject jthis, jlong jhandle) {
  API_BEGIN()
  const auto* tensor_ptr = reinterpret_cast<torch::Tensor*>(jhandle);
  jlongArray size = env->NewLongArray(tensor_ptr->dim());
  env->SetLongArrayRegion(size, 0, tensor_ptr->dim(), reinterpret_cast<const jlong*>(tensor_ptr->sizes().data()));
  return size;
  API_END_RETURN()
}

JNIEXPORT jint JNICALL Java_ai_djl_pytorch_jni_PyTorchLibrary_torchDType(JNIEnv* env, jobject jthis, jlong jhandle) {
  API_BEGIN()
  const auto* tensor_ptr = reinterpret_cast<torch::Tensor*>(jhandle);
  return utils::GetDTypeFromScalarType(tensor_ptr->scalar_type());
  API_END_RETURN()
}

JNIEXPORT jintArray JNICALL Java_ai_djl_pytorch_jni_PyTorchLibrary_torchDevice(
    JNIEnv* env, jobject jthis, jlong jhandle) {
  API_BEGIN()
  jclass jexception = env->FindClass("java/lang/NullPointerException");
  const auto* tensor_ptr = reinterpret_cast<torch::Tensor*>(jhandle);
  jintArray result = env->NewIntArray(2);
  if (result == nullptr) {
    env->ThrowNew(jexception, "Unable to create int array");
  }
  jint temp_device[] = {static_cast<int>(tensor_ptr->device().type()), tensor_ptr->device().index()};
  env->SetIntArrayRegion(result, 0, 2, temp_device);
  return result;
  API_END_RETURN()
}

JNIEXPORT jint JNICALL Java_ai_djl_pytorch_jni_PyTorchLibrary_torchLayout(JNIEnv* env, jobject jthis, jlong jhandle) {
  API_BEGIN()
  jclass jexception = env->FindClass("java/lang/IllegalStateException");
  const auto* tensor_ptr = reinterpret_cast<torch::Tensor*>(jhandle);
  auto layout = tensor_ptr->layout();
  switch (layout) {
    case torch::kStrided:
      return 0;
    case torch::kSparse:
      return 1;
    case torch::kMkldnn:
      return 2;
    default:
      env->ThrowNew(jexception, "Internal PyTorch error, layout should only have kStrided, kSparse or kMkldnn");
  }
  API_END_RETURN()
}

JNIEXPORT jlong JNICALL Java_ai_djl_pytorch_jni_PyTorchLibrary_torchTo(
    JNIEnv* env, jobject jthis, jlong jhandle, jint jdtype, jintArray jdevice, jboolean jcopy) {
  API_BEGIN()
  torch::NoGradGuard NoGradGuard;
  const auto* tensor_ptr = reinterpret_cast<torch::Tensor*>(jhandle);
  const auto device = utils::GetDeviceFromJDevice(env, jdevice);
  auto result = tensor_ptr->to(device, utils::GetScalarTypeFromDType(jdtype), false, jcopy == JNI_TRUE);
  const auto* result_ptr = new torch::Tensor(result);
  return reinterpret_cast<uintptr_t>(result_ptr);
  API_END_RETURN()
}

JNIEXPORT jlong JNICALL Java_ai_djl_pytorch_jni_PyTorchLibrary_tensorClone(JNIEnv* env, jobject jthis, jlong jhandle) {
  API_BEGIN()
  torch::NoGradGuard NoGradGuard;
  const auto* tensor_ptr = reinterpret_cast<torch::Tensor*>(jhandle);
  const auto* result_ptr = new torch::Tensor(tensor_ptr->clone());
  return reinterpret_cast<uintptr_t>(result_ptr);
  API_END_RETURN()
}

JNIEXPORT jlong JNICALL Java_ai_djl_pytorch_jni_PyTorchLibrary_torchIndex(JNIEnv* env, jobject jthis, jlong jhandle,
    jlongArray jmin_indices, jlongArray jmax_indices, jlongArray jstep_indices) {
  API_BEGIN()
  const auto* tensor_ptr = reinterpret_cast<torch::Tensor*>(jhandle);
  auto indices = utils::CreateTensorIndex(env, jmin_indices, jmax_indices, jstep_indices);
  const auto* result_ptr = new torch::Tensor(tensor_ptr->index(indices));
  return reinterpret_cast<uintptr_t>(result_ptr);
  API_END_RETURN()
}

JNIEXPORT void JNICALL Java_ai_djl_pytorch_jni_PyTorchLibrary_torchIndexPut(JNIEnv* env, jobject jthis, jlong jhandle,
    jlong jvalue_handle, jlongArray jmin_indices, jlongArray jmax_indices, jlongArray jstep_indices) {
  API_BEGIN()
  auto* tensor_ptr = reinterpret_cast<torch::Tensor*>(jhandle);
  const auto* value_ptr = reinterpret_cast<torch::Tensor*>(jvalue_handle);
  auto indices = utils::CreateTensorIndex(env, jmin_indices, jmax_indices, jstep_indices);
  tensor_ptr->index_put_(indices, *value_ptr);
  API_END()
}

JNIEXPORT void JNICALL Java_ai_djl_pytorch_jni_PyTorchLibrary_torchSet(
    JNIEnv* env, jobject jthis, jlong jself, jlong jreplace) {
  API_BEGIN()
  const auto* self_ptr = reinterpret_cast<torch::Tensor*>(jself);
  const auto* replace_ptr = reinterpret_cast<torch::Tensor*>(jreplace);
  self_ptr->set_(*replace_ptr);
  API_END()
}

JNIEXPORT jlong JNICALL Java_ai_djl_pytorch_jni_PyTorchLibrary_torchSlice(
    JNIEnv* env, jobject jthis, jlong jhandle, jlong jdim, jlong jstart, jlong jend, jlong jstep) {
  API_BEGIN()
  const auto* tensor_ptr = reinterpret_cast<torch::Tensor*>(jhandle);
  const auto* result_ptr = new torch::Tensor(tensor_ptr->slice(jdim, jstart, jend, jstep));
  return reinterpret_cast<uintptr_t>(result_ptr);
  API_END_RETURN()
}

JNIEXPORT jlong JNICALL Java_ai_djl_pytorch_jni_PyTorchLibrary_torchGather(
    JNIEnv* env, jobject jthis, jlong jhandle, jlong jindex_handle, jlong dim, jboolean sparse_grad) {
  API_BEGIN()
  const auto* tensor_ptr = reinterpret_cast<torch::Tensor*>(jhandle);
  const auto* index_ptr = reinterpret_cast<torch::Tensor*>(jindex_handle);
  const auto* result_ptr = new torch::Tensor(tensor_ptr->gather(dim, *index_ptr, sparse_grad));
  return reinterpret_cast<uintptr_t>(result_ptr);
  API_END_RETURN()
}

JNIEXPORT jlong JNICALL Java_ai_djl_pytorch_jni_PyTorchLibrary_torchMaskedSelect(
    JNIEnv* env, jobject jthis, jlong jhandle, jlong jmasked_handle) {
  API_BEGIN()
  const auto* tensor_ptr = reinterpret_cast<torch::Tensor*>(jhandle);
  const auto* index_ptr = reinterpret_cast<torch::Tensor*>(jmasked_handle);
  const auto* result_ptr = new torch::Tensor(tensor_ptr->masked_select(*index_ptr));
  return reinterpret_cast<uintptr_t>(result_ptr);
  API_END_RETURN()
}

JNIEXPORT void JNICALL Java_ai_djl_pytorch_jni_PyTorchLibrary_torchMaskedPut(
    JNIEnv* env, jobject jthis, jlong jhandle, jlong jvalue_handle, jlong jmasked_handle) {
  API_BEGIN()
  const auto* tensor_ptr = reinterpret_cast<torch::Tensor*>(jhandle);
  const auto* index_ptr = reinterpret_cast<torch::Tensor*>(jmasked_handle);
  const auto* value_ptr = reinterpret_cast<torch::Tensor*>(jvalue_handle);
  tensor_ptr->masked_fill_(*index_ptr, *value_ptr);
  API_END()
}

JNIEXPORT jbyteArray JNICALL Java_ai_djl_pytorch_jni_PyTorchLibrary_torchDataPtr(
    JNIEnv* env, jobject jthis, jlong jhandle) {
  API_BEGIN()
  const auto* tensor_ptr = reinterpret_cast<torch::Tensor*>(jhandle);
  // sparse and mkldnn are required to be converted to dense to access data ptr
  auto tensor = (tensor_ptr->is_sparse() || tensor_ptr->is_mkldnn()) ? tensor_ptr->to_dense() : *tensor_ptr;
  tensor = (tensor.is_contiguous()) ? tensor : tensor.contiguous();
  jbyteArray result = env->NewByteArray(tensor.nbytes());
  env->SetByteArrayRegion(result, 0, tensor.nbytes(), static_cast<const jbyte*>(tensor.data_ptr()));
  return result;
  API_END_RETURN()
}

JNIEXPORT void JNICALL Java_ai_djl_pytorch_jni_PyTorchLibrary_torchDeleteTensor(
    JNIEnv* env, jobject jthis, jlong jhandle) {
  API_BEGIN()
  const auto* tensor_ptr = reinterpret_cast<torch::Tensor*>(jhandle);
  delete tensor_ptr;
  API_END()
}

JNIEXPORT jlong JNICALL Java_ai_djl_pytorch_jni_PyTorchLibrary_torchToSparse(
    JNIEnv* env, jobject jthis, jlong jhandle) {
  API_BEGIN()
  torch::NoGradGuard NoGradGuard;
  const auto* tensor_ptr = reinterpret_cast<torch::Tensor*>(jhandle);
  const auto* result_ptr = new torch::Tensor(tensor_ptr->to_sparse());
  return reinterpret_cast<uintptr_t>(result_ptr);
  API_END_RETURN()
}

JNIEXPORT jlong JNICALL Java_ai_djl_pytorch_jni_PyTorchLibrary_torchToDense(JNIEnv* env, jobject jthis, jlong jhandle) {
  API_BEGIN()
  torch::NoGradGuard NoGradGuard;
  const auto* tensor_ptr = reinterpret_cast<torch::Tensor*>(jhandle);
  const auto* result_ptr = new torch::Tensor(tensor_ptr->to_dense());
  return reinterpret_cast<uintptr_t>(result_ptr);
  API_END_RETURN()
}

JNIEXPORT jboolean JNICALL Java_ai_djl_pytorch_jni_PyTorchLibrary_torchRequiresGrad(
    JNIEnv* env, jobject jthis, jlong jhandle) {
  API_BEGIN()
  const auto* tensor_ptr = reinterpret_cast<torch::Tensor*>(jhandle);
  return tensor_ptr->requires_grad();
  API_END_RETURN()
}

JNIEXPORT jstring JNICALL Java_ai_djl_pytorch_jni_PyTorchLibrary_torchGradFnName(
    JNIEnv* env, jobject jthis, jlong jhandle) {
  API_BEGIN()
  const auto* tensor_ptr = reinterpret_cast<torch::Tensor*>(jhandle);
  return env->NewStringUTF(tensor_ptr->grad_fn()->name().c_str());
  API_END_RETURN()
}

JNIEXPORT void JNICALL Java_ai_djl_pytorch_jni_PyTorchLibrary_torchAttachGrad(
    JNIEnv* env, jobject jthis, jlong jhandle) {
  API_BEGIN()
  const auto* tensor_ptr = reinterpret_cast<torch::Tensor*>(jhandle);
  tensor_ptr->requires_grad_(true);
  API_END()
}

JNIEXPORT jlong JNICALL Java_ai_djl_pytorch_jni_PyTorchLibrary_torchGrad(JNIEnv* env, jobject jthis, jlong jhandle) {
  API_BEGIN()
  const auto* tensor_ptr = reinterpret_cast<torch::Tensor*>(jhandle);
  const auto* result_ptr = new torch::Tensor(tensor_ptr->grad());
  if (!tensor_ptr->grad().defined()) {
    return utils::NULL_PTR;
  }
  return reinterpret_cast<uintptr_t>(result_ptr);
  API_END_RETURN()
}

JNIEXPORT jlong JNICALL Java_ai_djl_pytorch_jni_PyTorchLibrary_torchDetachGrad(
    JNIEnv* env, jobject jthis, jlong jhandle) {
  API_BEGIN()
  const auto* tensor_ptr = reinterpret_cast<torch::Tensor*>(jhandle);
  const auto* result_ptr = new torch::Tensor(tensor_ptr->detach());
  return reinterpret_cast<uintptr_t>(result_ptr);
  API_END_RETURN()
}

JNIEXPORT void JNICALL Java_ai_djl_pytorch_jni_PyTorchLibrary_torchBackward(
    JNIEnv* env, jobject jthis, jlong jhandle, jlong jgrad_handle, jboolean keep_graph, jboolean create_graph) {
  API_BEGIN()
  const auto* tensor_ptr = reinterpret_cast<torch::Tensor*>(jhandle);
  const auto* grad_ptr = reinterpret_cast<torch::Tensor*>(jgrad_handle);
  tensor_ptr->backward(*grad_ptr, keep_graph, create_graph);
  API_END()
}
