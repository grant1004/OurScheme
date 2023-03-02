# include <iostream> 
# include <cstdio> 

struct TreeNode {
    int value;
    TreeNode* left;
    TreeNode* right;
    TreeNode(int val) : value(val), left(NULL), right(NULL) {}
};

TreeNode* buildTreeFromLeaf(int val) {
    TreeNode* node = new TreeNode(val);
    if (val > 1) {
        node->left = buildTreeFromLeaf(val-1);
        node->right = buildTreeFromLeaf(val-2);
    }
    return node;
}

void inorderTraversal(TreeNode* node) {
    if (node == NULL) {
        return;
    }
    inorderTraversal(node->left);
    std::cout << node->value << " ";
    inorderTraversal(node->right);
}


void preorderTraversal(TreeNode* node) {
    if (node == NULL) {
        return;
    }
    std::cout << node->value << " ";
    preorderTraversal(node->left);
    preorderTraversal(node->right);
}

void postorderTraversal(TreeNode* node) {
    if (node == NULL) {
        return;
    }
    postorderTraversal(node->left);
    postorderTraversal(node->right);
    std::cout << node->value << " ";
}


int main() {
  
    TreeNode* root = buildTreeFromLeaf(7);
    
    std::cout << "Inorder traversal: ";
    inorderTraversal(root);
    std::cout << std::endl;
    
    std::cout << "Preorder traversal: ";
    preorderTraversal(root);
    std::cout << std::endl;
    
    std::cout << "Postorder traversal: ";
    postorderTraversal(root);
    std::cout << std::endl;
    
    // 釋放節點
    // deleteTree(root);
    return 0;
}